/* -*- c++ -*- */
/*
 * Copyright 2015 Ettus Research
 * Copyright 2020 Ettus Research, LLC. A National Instruments Brand
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
 */


#ifndef INCLUDED_ETTUS_FOSPHOR_DISPLAY_H
#define INCLUDED_ETTUS_FOSPHOR_DISPLAY_H

#include <gnuradio/sync_block.h>
#include <gnuradio/ettus/api.h>
#include <string>

class QApplication;
class QWidget;

namespace gr {

namespace ettus {

/*!
 * \brief QT GUI Display block for RFNoC fosphor
 * \ingroup ettus
 */
class ETTUS_API fosphor_display : virtual public gr::block
{
public:
    typedef std::shared_ptr<fosphor_display> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of ettus::fosphor_display
     */
    static sptr make(const int fft_bins = 256,
                     const int pwr_bins = 64,
                     const int wf_lines = 512,
                     QWidget* parent = nullptr);

    /* Block API */
    virtual void set_frequency_range(const double center_freq,
                                     const double samp_rate) = 0;
    virtual void set_waterfall(bool enabled) = 0;
    virtual void set_grid(bool enabled) = 0;
    virtual void set_palette(const std::string& name) = 0;
    virtual void set_frame_rate(int fps) = 0;

    /* QT GUI Widget stuff */
    virtual void exec_() = 0;
    virtual QWidget* qwidget() = 0;

#if defined(PY_VERSION) || defined(SWIGPYTHON)
    virtual PyObject* pyqwidget() = 0;
#else
    virtual void* pyqwidget() = 0;
#endif

    QApplication* d_qApplication;
};

} // namespace ettus

} // namespace gr

#endif /* INCLUDED_ETTUS_FOSPHOR_DISPLAY_H */

// vim: ts=2 sw=2 expandtab
