/* -*- c++ -*- */
/*
 * Copyright 2015 Ettus Research
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifdef ENABLE_PYTHON
#include <Python.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "fosphor_display_impl.h"

#include <QApplication>
#include <QWidget>
#include "QFosphorSurface.h"

#include <stdint.h>

namespace gr {
  namespace ettus {

    fosphor_display::sptr
    fosphor_display::make(const int fft_bins, const int pwr_bins, const int wf_lines, QWidget *parent)
    {
      return gnuradio::get_initial_sptr(new fosphor_display_impl(fft_bins, pwr_bins, wf_lines, parent));
    }

    fosphor_display_impl::fosphor_display_impl(const int fft_bins,
                                               const int pwr_bins,
                                               const int wf_lines,
                                               QWidget *parent)
      : gr::block("fosphor_display",
                  gr::io_signature::make(1, 2, fft_bins),
                  gr::io_signature::make(0, 0, 0)),
        d_fft_bins(fft_bins), d_pwr_bins(pwr_bins), d_wf_lines(wf_lines),
        d_center_freq(0.0), d_samp_rate(0.0), d_frame_rate(0),
        d_aligned(false), d_subframe(0), d_subframe_num(pwr_bins + 2)
    {
      /* Message Port output */
      message_port_register_out(pmt::mp("cfg"));

      /* Frame buffer */
      this->d_frame = new uint8_t[fft_bins * this->d_subframe_num];

      /* QT stuff */
      if(qApp != NULL) {
        this->d_qApplication = qApp;
      } else {
        int argc=0;
        char **argv = NULL;
        this->d_qApplication = new QApplication(argc, argv);
      }

      this->d_gui = new QFosphorSurface(fft_bins, pwr_bins, wf_lines, parent);
      this->d_gui->setFocusPolicy(Qt::StrongFocus);
      this->d_gui->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    fosphor_display_impl::~fosphor_display_impl()
    {
      delete this->d_frame;
    }


    void
    fosphor_display_impl::set_frequency_range(const double center_freq,
                                              const double samp_rate)
    {
      pmt::pmt_t msg = pmt::make_dict();

      if ((this->d_center_freq == center_freq) &&
          (this->d_samp_rate == samp_rate))
        return;

      if ((this->d_samp_rate != samp_rate) && (samp_rate > 0.0) && (this->d_frame_rate > 0))
      {
        int decim;

        decim = (int)(this->d_samp_rate / (this->d_fft_bins * this->d_pwr_bins * this->d_frame_rate));
        if (decim < 2)
          decim = 2;

        msg = pmt::dict_add(msg,
          pmt::string_to_symbol("decim"), pmt::from_long(decim)
        );
      }

      this->d_gui->setFrequencyRange(center_freq, samp_rate);

      this->d_center_freq = center_freq;
      this->d_samp_rate = samp_rate;

      msg = pmt::dict_add(msg,
        pmt::string_to_symbol("clear"), pmt::from_long(1)
      );

      message_port_pub(pmt::mp("cfg"), msg);
    }

    void
    fosphor_display_impl::set_waterfall(bool enabled)
    {
      this->d_gui->setWaterfall(enabled);
    }

    void
    fosphor_display_impl::set_grid(bool enabled)
    {
      this->d_gui->setGrid(enabled);
    }

    void
    fosphor_display_impl::set_palette(std::string name)
    {
      this->d_gui->setPalette(name);
    }

    void
    fosphor_display_impl::set_frame_rate(int fps)
    {
      this->d_frame_rate = fps;
    }


    bool
    fosphor_display_impl::start()
    {
      pmt::pmt_t msg = pmt::make_dict();

      {
        int decim;

        decim = (int)(this->d_samp_rate / (this->d_fft_bins * this->d_pwr_bins * this->d_frame_rate));
        if (decim < 2)
          decim = 2;

        msg = pmt::dict_add(msg,
          pmt::string_to_symbol("decim"), pmt::from_long(decim)
        );
      }

      msg = pmt::dict_add(msg,
        pmt::string_to_symbol("clear"), pmt::from_long(1)
      );

      message_port_pub(pmt::mp("cfg"), msg);
    }

    void
    fosphor_display_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* Need at least one item */
      ninput_items_required[0] = 1;

      /* Don't wait on WaterFall data */
      if (ninput_items_required.size() > 1)
        ninput_items_required[1] = 0;
    }

    int
    fosphor_display_impl::general_work(int noutput_items,
                                       gr_vector_int &ninput_items,
                                       gr_vector_const_void_star &input_items,
                                       gr_vector_void_star &output_items)
    {
      int rv;

      /* Consume histogram */
      rv = this->_work_hist(
        (const uint8_t *) input_items[0],
        ninput_items[0],
        0
      );

      if (rv > 0)
        consume(0, rv);

      /* Consume waterfall, if available */
      if (ninput_items.size() > 1)
      {
        rv = this->_work_wf(
          (const uint8_t *) input_items[1],
          ninput_items[1],
          1
        );

        if (rv > 0)
          consume(1, rv);
      }

      return 0;
    }

    int
    fosphor_display_impl::_work_hist(const uint8_t *input, int n_items, int port)
    {
      static const pmt::pmt_t EOB_KEY = pmt::string_to_symbol("rx_eob");
      std::vector<tag_t> v;
      uint64_t nR = nitems_read(port);
      int pEOF;

      /* Anything to do ? */
      if (n_items < 1)
        return 0;

      /* Grab all tags available, we'll need them either way */
      get_tags_in_range(v, port, nR, nR + n_items);

      /* If not aligned we just search for EOF */
      if (!this->d_aligned) {
        /* No tags ?  Drop all useless data while we wait for one */
        if (v.empty())
          return n_items;

        /* Start at zero right after the tag */
        this->d_subframe = 0;
        this->d_aligned = true;

        return v[0].offset - nR + 1;
      }

      /* Check alignement */
      pEOF = v.empty() ? -1 : (v[0].offset - nR);

      if (n_items >= (this->d_subframe_num - this->d_subframe))
      {
        /* We have the end of the frame, must be an EOF on that */
        if (pEOF != (this->d_subframe_num - this->d_subframe - 1))
        {
          this->d_aligned = false;
          return 0;
        }
      } else {
        /* We don't have the end of the frame yet, there can't be any EOF */
        if (pEOF >= 0) {
          this->d_aligned = false;
          return 0;
        }
      }

      /* Limit to expected frame boundary */
      if (n_items > (this->d_subframe_num - this->d_subframe))
        n_items = this->d_subframe_num - this->d_subframe;

      /* Copy the data */
      memcpy(
        &this->d_frame[this->d_fft_bins * this->d_subframe],
        input,
        n_items * this->d_fft_bins
      );

      this->d_subframe += n_items;

      /* Are we done ? */
      if (this->d_subframe == this->d_subframe_num)
      {
        /* Send the frame to the display surface */
        this->d_gui->sendFrame(this->d_frame, this->d_subframe_num * this->d_fft_bins);

        /* Start over */
        this->d_subframe = 0;
      }

      return n_items;
    }

    int
    fosphor_display_impl::_work_wf(const uint8_t *input, int n_items, int port)
    {
      /* Anything to do ? */
      if (n_items < 1)
        return 0;

      /* Send them to the display surface */
      this->d_gui->sendWaterfall(input, n_items);

      return n_items;
    }


    void
    fosphor_display_impl::exec_()
    {
      this->d_qApplication->exec();
    }

    QWidget*
    fosphor_display_impl::qwidget()
    {
      return dynamic_cast<QWidget*>(this->d_gui);
    }

#ifdef ENABLE_PYTHON
    PyObject*
    fosphor_display_impl::pyqwidget()
    {
      PyObject *w = PyLong_FromVoidPtr((void*)dynamic_cast<QWidget*>(this->d_gui));
      PyObject *retarg = Py_BuildValue("N", w);
      return retarg;
    }
#else
    void*
    fosphor_display_impl::pyqwidget()
    {
      return NULL;
    }
#endif

  }
}

// vim: ts=2 sw=2 expandtab
