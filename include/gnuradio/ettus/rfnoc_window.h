/* -*- c++ -*- */
/*
 * Copyright 2019 Ettus Research, a National Instruments Brand.
 * Copyright 2020 Ettus Research, A National Instruments Brand.
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

#ifndef INCLUDED_ETTUS_RFNOC_WINDOW_H
#define INCLUDED_ETTUS_RFNOC_WINDOW_H

#include <gnuradio/ettus/api.h>
#include <gnuradio/ettus/rfnoc_block.h>

namespace gr {
namespace ettus {

/*! RFNoC Window Block
 *
 * \ingroup uhd_blk
 */
class ETTUS_API rfnoc_window : virtual public rfnoc_block
{
public:
    typedef std::shared_ptr<rfnoc_window> sptr;

    /*!
     * \param graph Reference to the rfnoc_graph object this block is attached to
     * \param block_args Additional block arguments
     * \param device_select Device Selection
     * \param instance Instance Selection
     */
    static sptr make(rfnoc_graph::sptr graph,
                     const ::uhd::device_addr_t& block_args,
                     const int device_select,
                     const int instance);

    /*! Set the Window coefficients
     *
     * \param coeffs Vector of Coeffs (float)
     * \param chan   Channel Index
     */
    virtual void set_coefficients(const std::vector<float>& coeffs,
                                  const size_t chan) = 0;

    /*! Set the Window coefficients
     *
     * \param coeffs Vector of Coeffs (int16)
     * \param chan   Channel Index
     */
    virtual void set_coefficients(const std::vector<int16_t>& coeffs,
                                  const size_t chan) = 0;

    /*! Get the number of Window coefficients
     *
     * \param chan   Channel Index
     */
    virtual size_t get_max_num_coefficients(const size_t chan) = 0;

    /*! Returns a vector of Window coefficients
     *
     * \param chan   Channel Index
     */
    virtual std::vector<int16_t> get_coefficients(const size_t chan) = 0;
};

} // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_WINDOW_H */
