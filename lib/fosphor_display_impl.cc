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
    fosphor_display::make(const int fft_bins, const int pwr_bins, QWidget *parent)
    {
      return gnuradio::get_initial_sptr(new fosphor_display_impl(fft_bins, pwr_bins, parent));
    }
  
    fosphor_display_impl::fosphor_display_impl(const int fft_bins,
                                               const int pwr_bins,
                                               QWidget *parent)
      : gr::sync_block("fosphor_display",
                       gr::io_signature::make(1, 1, fft_bins),
                       gr::io_signature::make(0, 0, 0)),
        d_fft_bins(fft_bins), d_pwr_bins(pwr_bins),
        d_aligned(false), d_subframe(0), d_subframe_num(pwr_bins + 3) /* FIXME SYNC */
    {
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

      this->d_gui = new QFosphorSurface(fft_bins, pwr_bins, parent);
      this->d_gui->setFocusPolicy(Qt::StrongFocus);
      this->d_gui->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    fosphor_display_impl::~fosphor_display_impl()
    {
      delete this->d_frame;
    }


    int
    fosphor_display_impl::work (int noutput_items,
                                gr_vector_const_void_star &input_items,
                                gr_vector_void_star &output_items)
    {
      const uint8_t *input = (const uint8_t *) input_items[0];
      int i;

      // printf("%2d %d\n", noutput_items, this->d_aligned);

      if (noutput_items < 1)
        return 0;

      /* If not aligned we just search for EOF */
      if (!this->d_aligned) {
        for (i=0; i<this->d_fft_bins; i++)
         if (input[i] != 0xa5)
          break;

        if (i == this->d_fft_bins)
          this->d_aligned = true;

        return 1;
      }

      /* Limit to expected frame boundary */
      if (noutput_items > (this->d_subframe_num - this->d_subframe))
        noutput_items = this->d_subframe_num - this->d_subframe;

      /* Copy the data */
      memcpy(
        &this->d_frame[this->d_fft_bins * this->d_subframe],
        input,
        noutput_items * this->d_fft_bins
      );

      this->d_subframe += noutput_items;

      /* Are we done ? */
      if (this->d_subframe == this->d_subframe_num)
      {
        /* Check if the last subframe is EOF */
        for (i=0; i<this->d_fft_bins; i++)
         if (this->d_frame[(this->d_subframe - 1) * this->d_fft_bins + i] != 0xa5)
          break;

        if (i != this->d_fft_bins)
          this->d_aligned = false;

        /* Send the frame to the display surface */
        this->d_gui->sendFrame(this->d_frame, this->d_subframe_num * this->d_fft_bins);

        /* Start over */
        this->d_subframe = 0;
      }

      return noutput_items;
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
