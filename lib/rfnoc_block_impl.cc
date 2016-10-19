/* -*- c++ -*- */
/* Copyright 2015-2016 Ettus Research
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * gr-ettus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gr-ettus; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <ettus/rfnoc_block_impl.h>
#include <gnuradio/block_detail.h>

using namespace gr::ettus;

/****************************************************************************
 * Helper functions
 ****************************************************************************/
// Merges args1 and args2. Keys in excluded_keys keys are skipped.
// If a key is both in args1 and args2, that's fine as long as the value
// is the same. Otherwise, throw.
static ::uhd::device_addr_t merge_args(
    ::uhd::device_addr_t args1,
    ::uhd::device_addr_t args2,
    std::set< std::string > excluded_keys
) {
  ::uhd::device_addr_t merged_args = args1;
  BOOST_FOREACH(const std::string &key, args2.keys()) {
    if (excluded_keys.count(key)) {
      continue;
    }
    if (!merged_args.has_key(key)) {
      merged_args[key] = args2[key];
    } else if (merged_args[key] != args2[key]) {
      throw std::runtime_error(str(
          boost::format("Error merging arguments - conflicting values: '%s': '%s' != '%s': '%s'")
          % key % merged_args[key] % key % args2[key]
      ));
    }
  }

  return merged_args;
}

// Read stream args and block stream signature, returns itemsize
// and vlen.
static void set_signature_from_block(
    ::uhd::stream_args_t stream_args,
    ::uhd::rfnoc::block_ctrl_base::sptr blk_ctrl,
    size_t &itemsize,
    size_t &nchans,
    size_t &vlen,
    bool tx
) {
  // If the pointer is not set, the block does not support
  // this direction (tx or rx)
  if (!blk_ctrl) {
    itemsize = 0;
    nchans   = 0;
    vlen     = 0;
    return;
  }

  // Otherwise, set itemsize...
  const std::string dir = tx ? "TX" : "RX";
  if (stream_args.cpu_format.empty()) {
    throw std::runtime_error(str(
        boost::format("%s: No cpu_format specified on %s.")
        % blk_ctrl->unique_id() % dir
    ));
  }
  itemsize = ::uhd::convert::get_bytes_per_item(stream_args.cpu_format);
  // ...and vector length:
  int gr_vlen = stream_args.args.cast<int>("gr_vlen", -1);
  size_t block_port = 0; // TODO moar options
  ::uhd::rfnoc::stream_sig_t sig = tx
        ? boost::dynamic_pointer_cast< ::uhd::rfnoc::sink_block_ctrl_base >(blk_ctrl)->get_input_signature(block_port)
        : boost::dynamic_pointer_cast< ::uhd::rfnoc::source_block_ctrl_base >(blk_ctrl)->get_output_signature(block_port);
  vlen = (sig.vlen == 0) ? 1 : sig.vlen;
  if (gr_vlen != -1) {
    if (gr_vlen == 1) {
      // This is a way to force scalar streams even if the upstream
      // block is setting vlens
      vlen = 1;
    } else if (vlen != 1 && vlen != gr_vlen) {
      // In all other cases, if there's a mismatch, we throw rather
      // than trying to match vector lengths.
      throw std::runtime_error(str(boost::format("Can't set gr_vlen to %d if underlying RFNoC block already has a vector length of %d.\n") % gr_vlen % vlen));
    }
    vlen = gr_vlen;
  }

  return;
}

/*********************************************************************
 * Statics
 *********************************************************************/
std::string rfnoc_block_impl::make_block_id(
    const std::string &block_name,
    const int block_select,
    const int device_select
) {
  std::string dev_str;
  if (device_select >= 0) {
    dev_str = str(boost::format("%d/") % device_select);
  }
  std::string block_str;
  if (block_select >= 0) {
    block_str = str(boost::format("_%d") % block_select);
  }

  return str(boost::format("%s%s%s") % dev_str % block_name % block_str);
}


/**********************************************************************
* Structors
*********************************************************************/
rfnoc_block::rfnoc_block(const std::string &name) :
    gr::block(name,
        /* Default IO signatures: These will be overridden later */
        gr::io_signature::make(0, 0, 1),
        gr::io_signature::make(0, 0, 1)
    )
{
    // nop
}

rfnoc_block_impl::rfnoc_block_impl(
    const device3::sptr &dev,
    const std::string &block_id,
    const ::uhd::stream_args_t &tx_stream_args,
    const ::uhd::stream_args_t &rx_stream_args
) : _dev(dev->get_device()),
    _tx(tx_stream_args),
    _rx(rx_stream_args)
{
  std::vector< ::uhd::rfnoc::block_id_t > blocks =
          dev->get_device()->find_blocks(block_id);
  if (blocks.empty()) {
      throw std::runtime_error(str(boost::format("Cannot find a block for ID: %s") % block_id));
  }
  _blk_ctrl = dev->get_device()->get_block_ctrl(blocks.front());
  UHD_ASSERT_THROW(_blk_ctrl);

  // Configure the block
  const std::set< std::string > excluded_keys = boost::assign::list_of("align")("gr_vlen");
  _merged_args = merge_args(tx_stream_args.args, rx_stream_args.args, excluded_keys);
  if (_merged_args.size()) {
      GR_LOG_INFO(d_debug_logger, str(boost::format("Setting args on %s (%s)") % _blk_ctrl->get_block_id() % _merged_args.to_string()));
      _blk_ctrl->set_args(_merged_args);
  }

  _tx.stream_args.args["block_id"] = _blk_ctrl->get_block_id().get();
  _rx.stream_args.args["block_id"] = _blk_ctrl->get_block_id().get();

  //Each block can have multiple tx/rx channels. This works when channels are aligned.
  BOOST_FOREACH(const size_t chan_idx, _tx.stream_args.channels) {
    _tx.stream_args.args[str(boost::format("block_port%d") % chan_idx)] = str(boost::format("%d") % chan_idx);
  }
  BOOST_FOREACH(const size_t chan_idx, _rx.stream_args.channels) {
    _rx.stream_args.args[str(boost::format("block_port%d") % chan_idx)] = str(boost::format("%d") % chan_idx);
  }

  //// Final configuration for the GNU Radio block:
  set_tag_propagation_policy(TPP_DONT);
  update_gr_io_signature();

  // Add message ports
  message_port_register_in(pmt::mp("rfnoc"));
  set_msg_handler(
    pmt::mp("rfnoc"),
    boost::bind(&rfnoc_block_impl::handle_rfnoc_msg, this, _1)
  );
}

rfnoc_block_impl::~rfnoc_block_impl()
{
    // nop
}


/****************************************************************************
 * Static members
 ****************************************************************************/
std::map<std::string, bool> rfnoc_block_impl::_active_streamers;
reusable_barrier rfnoc_block_impl::_tx_barrier;
reusable_barrier rfnoc_block_impl::_rx_barrier;
boost::recursive_mutex rfnoc_block_impl::s_setup_mutex;

/*********************************************************************
 * GR Block functions
 *********************************************************************/
bool rfnoc_block_impl::check_topology(int ninputs, int noutputs)
{
  GR_LOG_DEBUG(d_debug_logger, str(boost::format("check_topology()")));
  { // scope the mutex:
    boost::recursive_mutex::scoped_lock lock(s_setup_mutex);
    std::string blk_id = get_block_id();
    if (ninputs || noutputs) {
      _active_streamers[blk_id] = true;
    } else if (_active_streamers.count(blk_id)) {
      _active_streamers.erase(blk_id);
    }
    GR_LOG_DEBUG(d_debug_logger, str(boost::format("RFNoC blocks with streaming ports: %d") % _active_streamers.size()));
    _tx_barrier.resize(_active_streamers.size());
    _rx_barrier.resize(_active_streamers.size());
  }

  // TODO: Check if ninputs and noutputs match the blocks io signatures.
  return true;
}

bool rfnoc_block_impl::start()
{
  boost::recursive_mutex::scoped_lock lock(d_mutex);
  size_t ninputs  = detail()->ninputs();
  size_t noutputs = detail()->noutputs();
  GR_LOG_DEBUG(d_debug_logger, str(boost::format("start(): ninputs == %d noutputs == %d") % ninputs % noutputs));

  if (ninputs == 0 && noutputs == 0) {
      return true;
  }

  // If the topology changed, we need to clear the old streamers
  if (_rx.streamers.size() != noutputs) {
    _rx.streamers.clear();
  }
  if (_tx.streamers.size() != ninputs) {
    _tx.streamers.clear();
  }

  //////////////////// TX ///////////////////////////////////////////////////////////////
  // Setup TX streamer.
  if (ninputs && _tx.streamers.empty()) {
    // Get a block control for the tx side:
    ::uhd::rfnoc::sink_block_ctrl_base::sptr tx_blk_ctrl =
        boost::dynamic_pointer_cast< ::uhd::rfnoc::sink_block_ctrl_base >(_blk_ctrl);
    if (!tx_blk_ctrl) {
      GR_LOG_FATAL(d_logger, str(boost::format("Not a sink_block_ctrl_base: %s") % _blk_ctrl->unique_id()));
      return false;
    }
    if (_tx.align) { // Aligned streamers:
      GR_LOG_DEBUG(d_debug_logger, str(boost::format("Creating one aligned tx streamer for %d inputs.") % ninputs));
      GR_LOG_DEBUG(d_debug_logger,
          str(boost::format("cpu: %s  otw: %s  args: %s channels.size: %d ") % _tx.stream_args.cpu_format % _tx.stream_args.otw_format % _tx.stream_args.args.to_string() % _tx.stream_args.channels.size()));
      assert(ninputs == _tx.stream_args.channels.size());
      ::uhd::tx_streamer::sptr tx_stream = _dev->get_tx_stream(_tx.stream_args);
      if (tx_stream) {
        _tx.streamers.push_back(tx_stream);
      } else {
        GR_LOG_FATAL(d_logger, str(boost::format("Can't create tx streamer(s) to: %s") % _blk_ctrl->get_block_id().get()));
        return false;
      }
    } else { // Unaligned streamers:
      for (size_t i = 0; i < size_t(ninputs); i++) {
        _tx.stream_args.channels = std::vector<size_t>(1, i);
        _tx.stream_args.args["block_port"] = str(boost::format("%d") % i);
        GR_LOG_DEBUG(d_debug_logger, str(boost::format("creating tx streamer with: %s") % _tx.stream_args.args.to_string()));
        ::uhd::tx_streamer::sptr tx_stream = _dev->get_tx_stream(_tx.stream_args);
        if (tx_stream) {
          _tx.streamers.push_back(tx_stream);
        }
      }
      if (_tx.streamers.size() != size_t(ninputs)) {
        GR_LOG_FATAL(d_logger, str(boost::format("Can't create tx streamer(s) to: %s") % _blk_ctrl->get_block_id().get()));
        return false;
      }
    }
  }

  _tx.metadata.start_of_burst = false;
  _tx.metadata.end_of_burst = false;
  _tx.metadata.has_time_spec = false;

  // Wait for all RFNoC streamers to have set up their tx streamers
  _tx_barrier.wait();

  //////////////////// RX ///////////////////////////////////////////////////////////////
  // Setup RX streamer
  if (noutputs && _rx.streamers.empty()) {
    // Get a block control for the rx side:
    ::uhd::rfnoc::source_block_ctrl_base::sptr rx_blk_ctrl =
        boost::dynamic_pointer_cast< ::uhd::rfnoc::source_block_ctrl_base >(_blk_ctrl);
    if (!rx_blk_ctrl) {
      GR_LOG_FATAL(d_logger, str(boost::format("Not a source_block_ctrl_base: %s") % _blk_ctrl->unique_id()));
      return false;
    }
    if (_rx.align) { // Aligned streamers:
      GR_LOG_DEBUG(d_debug_logger, str(boost::format("Creating one aligned rx streamer for %d outputs.") % noutputs));
      GR_LOG_DEBUG(d_debug_logger,
          str(boost::format("cpu: %s  otw: %s  args: %s channels.size: %d ") % _rx.stream_args.cpu_format % _rx.stream_args.otw_format % _rx.stream_args.args.to_string() % _rx.stream_args.channels.size()));
      assert(noutputs == _rx.stream_args.channels.size());
      ::uhd::rx_streamer::sptr rx_stream = _dev->get_rx_stream(_rx.stream_args);
      if (rx_stream) {
        _rx.streamers.push_back(rx_stream);
      } else {
        GR_LOG_FATAL(d_logger, str(boost::format("Can't create rx streamer(s) to: %s") % _blk_ctrl->get_block_id().get()));
        return false;
      }
    } else { // Unaligned streamers:
      for (size_t i = 0; i < size_t(noutputs); i++) {
        _rx.stream_args.channels = std::vector<size_t>(1, i);
        _rx.stream_args.args["block_port"] = str(boost::format("%d") % i);
        GR_LOG_DEBUG(d_debug_logger, str(boost::format("creating rx streamer with: %s") % _rx.stream_args.args.to_string()));
        ::uhd::rx_streamer::sptr rx_stream = _dev->get_rx_stream(_rx.stream_args);
        if (rx_stream) {
          _rx.streamers.push_back(rx_stream);
        }
      }
      if (_rx.streamers.size() != size_t(noutputs)) {
        GR_LOG_FATAL(d_logger, str(boost::format("Can't create rx streamer(s) to: %s") % _blk_ctrl->get_block_id().get()));
        return false;
      }
    }
  }

  // Wait for all RFNoC streamers to have set up their rx streamers
  _rx_barrier.wait();

  // Start the streamers
  if (!_rx.streamers.empty()) {
    ::uhd::stream_cmd_t stream_cmd(::uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
    stream_cmd.stream_now = true;
    for (size_t i = 0; i < _rx.streamers.size(); i++) {
      _rx.streamers[i]->issue_stream_cmd(stream_cmd);
    }
  }

  return true;
}

bool rfnoc_block_impl::stop()
{
  boost::recursive_mutex::scoped_lock lock(d_mutex);

  size_t ninputs  = detail()->ninputs();
  size_t noutputs = detail()->noutputs();
  GR_LOG_DEBUG(d_debug_logger, str(boost::format("start(): ninputs == %d noutputs == %d") % ninputs % noutputs));

  if (ninputs == 0 && noutputs == 0) {
      return true;
  }

  // TX: Send EOB
  _tx.metadata.start_of_burst = false;
  _tx.metadata.end_of_burst = true;
  _tx.metadata.has_time_spec = false;
  // TODO: One loop is enough here
  if (_tx.align) {
    _tx.streamers[0]->send(gr_vector_const_void_star(_tx.streamers[0]->get_num_channels()), 0, _tx.metadata, 1.0);
  } else {
    for (size_t i = 0; i < _tx.streamers.size(); i++) {
      // Always 1 channel per streamer in this case
      _tx.streamers[i]->send(gr_vector_const_void_star(1), 0, _tx.metadata, 1.0);
    }
  }

  _tx_barrier.wait();

  // RX: Stop streaming and empty the buffers
  for (size_t i = 0; i < _rx.streamers.size(); i++) {
    _rx.streamers[i]->issue_stream_cmd(::uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);
    flush(i);
  }

  return true;
}

void rfnoc_block_impl::flush(size_t streamer_index)
{
  const size_t nbytes = 4096;
  size_t nchan = 1;
  if (_rx.align) {
    nchan = _rx.streamers[0]->get_num_channels();
  }
  std::vector<std::vector<char> > buffs(nchan, std::vector<char>(nbytes));
  gr_vector_void_star outputs;
  for(size_t i = 0; i < nchan; i++) {
    outputs.push_back(&buffs[i].front());
  }
  while(true) {
    const size_t bpi = ::uhd::convert::get_bytes_per_item(_rx.stream_args.cpu_format);
    _rx.streamers[streamer_index]->recv(outputs, nbytes/bpi, _rx.metadata, 0.0);
    if(_rx.metadata.error_code != ::uhd::rx_metadata_t::ERROR_CODE_NONE) {
      break;
    }
  }
}

/*********************************************************************
 * I/O Signatures
 ********************************************************************/
void rfnoc_block_impl::update_rfnoc_io_signature()
{
  set_signature_from_block(
      _tx.stream_args,
      get_block_ctrl< ::uhd::rfnoc::sink_block_ctrl_base >(),
      _tx.itemsize, _tx.nchans, _tx.vlen,
      true
  );
  // Output signature / Rx:
  set_signature_from_block(
      _rx.stream_args,
      get_block_ctrl< ::uhd::rfnoc::source_block_ctrl_base >(),
      _rx.itemsize, _rx.nchans, _rx.vlen,
      false
  );
}

void rfnoc_block_impl::update_gr_io_signature()
{
  update_rfnoc_io_signature();
  set_input_signature(get_rfnoc_input_signature());
  set_output_signature(get_rfnoc_output_signature());
}

gr::io_signature::sptr rfnoc_block_impl::get_rfnoc_input_signature()
{
  const int min_streams = 0;
  const int max_streams = gr::io_signature::IO_INFINITE; // TODO
  return gr::io_signature::make(min_streams, max_streams, _tx.itemsize * _tx.vlen);
}

gr::io_signature::sptr rfnoc_block_impl::get_rfnoc_output_signature()
{
  const int min_streams = 0;
  const int max_streams = gr::io_signature::IO_INFINITE; // TODO
  GR_LOG_DEBUG(d_debug_logger, str(boost::format("output item size: %d") % (_rx.itemsize * _rx.vlen)));
  return gr::io_signature::make(min_streams, max_streams, _rx.itemsize * _rx.vlen);
}

/*********************************************************************
 * Streaming
 *********************************************************************/
int
rfnoc_block_impl::general_work (
    int noutput_items,
    gr_vector_int &ninput_items,
    gr_vector_const_void_star &input_items,
    gr_vector_void_star &output_items
) {
  boost::recursive_mutex::scoped_lock lock(d_mutex);
  // These call consume()
  if (!input_items.empty()) {
    if (_tx.align) {
      work_tx_a(ninput_items, input_items);
    } else {
      work_tx_u(ninput_items, input_items);
    }
  }

  // These call produce()
  if (!output_items.empty()) {
    if (_rx.align) {
      // Well, this doesn't
      return work_rx_a(noutput_items, output_items);
    } else {
      work_rx_u(noutput_items, output_items);
    }
  }

  return gr::block::WORK_CALLED_PRODUCE;
}

void
rfnoc_block_impl::work_tx_a(
    gr_vector_int &ninput_items,
    gr_vector_const_void_star &input_items
) {
  // TODO Figure out why this doesn't work. It looks like the fragmentation logic
  //  in the tx_streamer is screwing up the packets, they're definitely wrong
  //  on the wire (checked with wireshark).
  //size_t num_vectors_to_send = ninput_items[0];
  size_t num_vectors_to_send = _tx.streamers[0]->get_max_num_samps() / _rx.vlen;
  const size_t num_sent = _tx.streamers[0]->send(
      input_items,
      num_vectors_to_send * _tx.vlen,
      _tx.metadata,
      1.0
  );

  consume_each(num_sent / _tx.vlen);
}

void
rfnoc_block_impl::work_tx_u(
    gr_vector_int &ninput_items,
    gr_vector_const_void_star &input_items
) {
  assert(input_items.size() == _tx.streamers.size());

  // In every loop iteration, this will point to the relevant buffer
  gr_vector_const_void_star buff_ptr(1);

  for (size_t i = 0; i < _tx.streamers.size(); i++) {
    buff_ptr[0] = input_items[i];
    //size_t num_vectors_to_send = std::min(_tx.streamers[i]->get_max_num_samps() / _rx.vlen, size_t(ninput_items[i]));
    size_t num_vectors_to_send = ninput_items[i];
    const size_t num_sent = _tx.streamers[i]->send(
        buff_ptr,
        num_vectors_to_send * _tx.vlen,
        _tx.metadata,
        1.0
    );
    consume(i, num_sent / _tx.vlen);
  }
}

int
rfnoc_block_impl::work_rx_a(
    int noutput_items,
    gr_vector_void_star &output_items
) {
  size_t num_vectors_to_recv = noutput_items;
  size_t num_samps = _rx.streamers[0]->recv(
      output_items,
      num_vectors_to_recv * _rx.vlen,
      //_rx.metadata, 0.1, true
      _rx.metadata, 0.1
  );

  switch(_rx.metadata.error_code) {
    case ::uhd::rx_metadata_t::ERROR_CODE_NONE:
      break;

    case ::uhd::rx_metadata_t::ERROR_CODE_TIMEOUT:
      //its ok to timeout, perhaps the user is doing finite streaming
      std::cout << "timeout on chan 0" << std::endl;
      break;

    case ::uhd::rx_metadata_t::ERROR_CODE_OVERFLOW:
      // Not much we can do about overruns here
      std::cout << "overrun on chan 0" << std::endl;
      break;

    default:
      std::cout << boost::format("RFNoC Streamer block received error %s (Code: 0x%x)")
        % _rx.metadata.strerror() % _rx.metadata.error_code << std::endl;
  }

  if (_rx.metadata.end_of_burst) {
      for (size_t i = 0; i < output_items.size(); i++) {
          add_item_tag(
              i,
              nitems_written(i) + (num_samps / _rx.vlen) - 1,
              EOB_KEY, pmt::PMT_T
            );
      }
  }

  // There's no 'produce_each()', unfortunately
  return num_samps / _rx.vlen;
}

void
rfnoc_block_impl::work_rx_u(
    int noutput_items,
    gr_vector_void_star &output_items
) {
  assert(_rx.streamers.size() == output_items.size());

  // In every loop iteration, this will point to the relevant buffer
  gr_vector_void_star buff_ptr(1);

  for (size_t i = 0; i < _rx.streamers.size(); i++) {
    buff_ptr[0] = output_items[i];
    //size_t num_vectors_to_recv = std::min(_rx.streamers[i]->get_max_num_samps() / _rx.vlen, size_t(noutput_items));
    size_t num_vectors_to_recv = noutput_items;
    size_t num_samps = _rx.streamers[i]->recv(
        buff_ptr,
        num_vectors_to_recv * _rx.vlen,
        _rx.metadata, 0.1, true
    );

    switch(_rx.metadata.error_code) {
      case ::uhd::rx_metadata_t::ERROR_CODE_NONE:
        break;

      case ::uhd::rx_metadata_t::ERROR_CODE_TIMEOUT:
        //its ok to timeout, perhaps the user is doing finite streaming
        std::cout << "timeout on chan " << i << std::endl;
        break;

      case ::uhd::rx_metadata_t::ERROR_CODE_OVERFLOW:
        // Not much we can do about overruns here
        std::cout << "overrun on chan " << i << std::endl;
        break;

      default:
        std::cout << boost::format("RFNoC Streamer block received error %s (Code: 0x%x)")
          % _rx.metadata.strerror() % _rx.metadata.error_code << std::endl;
    }

    if (_rx.metadata.end_of_burst) {
      for (size_t i = 0; i < output_items.size(); i++) {
        add_item_tag(
            i,
            nitems_written(i) + (num_samps / _rx.vlen) - 1,
            EOB_KEY, pmt::PMT_T
        );
      }
    }

    produce(i, num_samps / _rx.vlen);
  } /* end for (chans) */
}

/*********************************************************************
 * Message handling
 *********************************************************************/
void rfnoc_block_impl::handle_rfnoc_msg(pmt::pmt_t msg)
{
  /* If the PMT is a list, assume it's a list of pairs and recurse for each */
  /* Works for dict too */
  try {
    /* Because of PMT is just broken you and can't distinguish between
     * pair and dict, we have to call length() and see if it will throw
     * or not ... */
    if (pmt::length(msg) > 0) {
      for (size_t i = 0; i < pmt::length(msg); i++) {
        this->handle_rfnoc_msg(pmt::nth(i, msg));
      }
      return;
    }
  }
  catch(...) {
    // nop (means it wasn't a dict)
  }

  /* Handle the pairs */
  if (pmt::is_pair(msg)) {
    pmt::pmt_t key(pmt::car(msg));
    pmt::pmt_t val(pmt::cdr(msg));
    if (!pmt::is_symbol(key)) {
      return;
    }

    try {
        const std::string key_str = pmt::symbol_to_string(key);
        if (pmt::is_bool(val)) {
          this->set_arg(key_str, pmt::to_bool(val));
        } else if (pmt::is_integer(val)) {
          this->set_arg(key_str, int(pmt::to_long(val)));
        } else if (pmt::is_real(val)) {
          this->set_arg(key_str, pmt::to_double(val));
        } else if (pmt::is_symbol(val)) {
          this->set_arg(key_str, pmt::symbol_to_string(val));
        }
        // TODO: Add vectors
    }
    catch (...) {
        GR_LOG_ERROR(d_logger, boost::format("Cannot set RFNoC block argument '%s'") % key);
    }
  }
}

