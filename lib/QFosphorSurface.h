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

#ifndef INCLUDED_ETTUS_QFOSPHORSURFACE_H
#define INCLUDED_ETTUS_QFOSPHORSURFACE_H

#include <QGLWidget>
#include <QGLFunctions>

#include <string>

extern "C" {
#include "axis.h"
}

namespace gr {
  namespace ettus {

    class QFosphorColorMapper;

    class QFosphorSurface : public ::QGLWidget, protected ::QGLFunctions
    {
      Q_OBJECT

     protected:
      void initializeGL();
      void resizeGL(int width, int height);
      void paintGL();

     public:
      QFosphorSurface(int fft_bins, int pwr_bins, int wf_lines, QWidget *parent);
      virtual ~QFosphorSurface();

      void setFrequencyRange(const double center_freq, const double span);
      void setWaterfall(bool enabled);
      void setGrid(bool enabled);
      void setPalette(std::string name);
      void sendFrame(void *frame, int frame_len);
      void sendWaterfall(const uint8_t *wf, int n);

     private:
      void drawHistogram();
      void drawHistogramIntensityScale();
      void drawSpectrum();
      void drawGrid();
      void drawWaterfall();
      void drawWaterfallIntensityScale();
      void drawMargins();
      void uploadFrameData();
      void uploadWaterfallData();
      void refreshPowerAxis();
      void refreshFrequencyAxis();
      void refreshLayout();

      int fft_bins;
      int pwr_bins;
      int wf_lines;

      bool grid_enabled;
      std::string palette;

      struct {
        bool dirty;
        void *data;
        GLuint tex;
        GLuint vbo;
        float *vbo_buf;
      } frame;

      struct {
        bool dirty;
        int pos;
        uint8_t *data_buf;
        GLuint tex;
      } wf;

      struct {
        bool  dirty;
        int   width;
        int   height;

        bool  wf_enabled;

        float x[4];
        float y[6];
        float x_div;
        float y_div;

        GLuint pwr_tex;
        GLuint freq_tex;
      } layout;

      QFosphorColorMapper *cmap;

      struct freq_axis freq_axis;
    };

  } // namespace fosphor
} // namespace gr

#endif /* INCLUDED_ETTUS_QFOSPHORSURFACE_H */

// vim: ts=2 sw=2 expandtab
