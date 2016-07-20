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
 */


#ifndef INCLUDED_ETTUS_FOSPHOR_DISPLAY_IMPL_H
#define INCLUDED_ETTUS_FOSPHOR_DISPLAY_IMPL_H

#include <ettus/fosphor_display.h>

#include <stdint.h>

namespace gr {
  namespace ettus {

    class QFosphorSurface;

    /*!
     * \brief QT GUI Display block for RFNoC fosphor (implementation)
     * \ingroup ettus
     */
    class ETTUS_API fosphor_display_impl : public fosphor_display
    {
     public:
      fosphor_display_impl(const int fft_bins = 256,
                           const int pwr_bins = 64,
                           const int wf_lines = 512,
                           QWidget *parent=NULL);
      ~fosphor_display_impl();

      /* Block API */
      void set_frequency_range(const double center_freq,
                               const double samp_rate);
      void set_waterfall(bool enabled);
      void set_grid(bool enabled);
      void set_palette(std::string name);
      void set_frame_rate(int fps);

      /* gr::block implementation */
      bool start();

      void forecast(int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);

      /* QT GUI Widget stuff */
      void exec_();
      QWidget* qwidget();

#if defined(PY_VERSION) || defined(SWIGPYTHON)
      PyObject* pyqwidget();
#else
      void* pyqwidget();
#endif

     private:
      int _work_hist(const uint8_t *input, int n_items, int port);
      int _work_wf(const uint8_t *input, int n_items, int port);

      QFosphorSurface *d_gui;

      int d_fft_bins;
      int d_pwr_bins;
      int d_wf_lines;

      double d_center_freq;
      double d_samp_rate;
      int d_frame_rate;

      bool d_aligned;
      int d_subframe;
      int d_subframe_num;
      uint8_t *d_frame;
    };

  } // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_FOSPHOR_DISPLAY_IMPL_H */

// vim: ts=2 sw=2 expandtab
