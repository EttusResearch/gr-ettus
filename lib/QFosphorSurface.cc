/* -*- c++ -*- */
/*
 * Copyright 2013-2015 Sylvain Munaut
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

#include "QFosphorSurface.h"
#include "QFosphorColorMapper.h"

#include <stdio.h>
#include <string.h>

#include <QPixmap>
#include <QPainter>
#include <QFont>


namespace gr {
  namespace ettus {

    QFosphorSurface::QFosphorSurface(int fft_bins, int pwr_bins, QWidget *parent)
      : QGLWidget(parent),
        fft_bins(fft_bins), pwr_bins(pwr_bins),
        palette("iron")
    {
      this->setFocusPolicy(Qt::StrongFocus);
      this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

      memset(&this->frame, 0, sizeof(this->frame));
      memset(&this->layout, 0, sizeof(this->layout));

      this->setFrequencyRange(0.0, 0.0);
    }


    /* -------------------------------------------------------------------- */
    /* Overloaded GL functions                                              */
    /* -------------------------------------------------------------------- */

    void
    QFosphorSurface::initializeGL()
    {
      /* Init frame texture */
      glGenTextures(1, &this->frame.tex);

      glBindTexture(GL_TEXTURE_2D, this->frame.tex);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

      /* Color map */
      this->cmap = new QFosphorColorMapper(this);
    }

    void
    QFosphorSurface::resizeGL(int width, int height)
    {
      /* Setup matrix to map GL coord to exact pixels */
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0.0, (double)width, 0.0, (double)height, -1.0, 1.0);

      /* Viewport on the whole area */
      glViewport(0, 0, width, height);

      /* Update layout */
      this->layout.width  = width;
      this->layout.height = height;
      this->layout.dirty  = true;
    }

    void
    QFosphorSurface::paintGL()
    {
      float x[2], y[2];
      int i;

      /* If no data, abort early */
      if (!this->frame.data)
        return;

      /* Upload texture if needed */
//      if (this->frame.dirty)
      {
        this->frame.dirty = false;

        glBindTexture(GL_TEXTURE_2D, this->frame.tex);

        glTexImage2D(
            GL_TEXTURE_2D,
            0, GL_R8,
            this->fft_bins, this->pwr_bins, 0,
            GL_RED, GL_UNSIGNED_BYTE,
            this->frame.data
        );
      }

      /* Refresh layout if needed */
      if (this->layout.dirty)
      {
        this->refreshLayout();
      }

      /* Clear everything */
      glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
      glClear(GL_COLOR_BUFFER_BIT);

      /* Draw Histogram texture */
      this->cmap->enable(this->palette, this->frame.tex);

      x[0] = this->layout.x[0];
      x[1] = this->layout.x[1];
      y[0] = this->layout.y[0];
      y[1] = this->layout.y[1];

      glBegin( GL_QUADS );
      glTexCoord2d(0.0f, -0.038f); glVertex2d(x[0], y[0]);
      glTexCoord2d(1.0f, -0.038f); glVertex2d(x[1], y[0]);
      glTexCoord2d(1.0f,  1.000f); glVertex2d(x[1], y[1]);
      glTexCoord2d(0.0f,  1.000f); glVertex2d(x[0], y[1]);
      glEnd();

      this->cmap->disable();

      /* Draw grid */
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glBegin(GL_LINES);
      glColor4f(0.02f, 0.02f, 0.02f, 0.5f);

        /* Vertical div */
      x[0] = this->layout.x[0] + 0.5f;
      x[1] = this->layout.x[1] - 0.5f;

      for (i=0; i<11; i++)
      {
        y[0] = this->layout.y[0] + i * this->layout.y_div + 0.5f;

        glVertex2f(x[0], y[0]);
        glVertex2f(x[1], y[0]);
      }

        /* Horizontal div */
      y[0] = this->layout.y[0] + 0.5f;
      y[1] = this->layout.y[1] - 0.5f;

      for (i=0; i<11; i++)
      {
        x[0] = this->layout.x[0] + i * this->layout.x_div + 0.5f;

        glVertex2f(x[0], y[0]);
        glVertex2f(x[0], y[1]);
      }

      glEnd();
      glDisable(GL_BLEND);

      /* Draw intensity scale */
      this->cmap->drawScale(this->palette,
        this->layout.x[1] +  2.0f,
        this->layout.y[0],
        this->layout.x[1] + 10.0f,
        this->layout.y[1]
      );

      /* Draw margins */
      glActiveTexture(GL_TEXTURE0);
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);

      x[0] = 0;
      y[0] = 0;

      glBindTexture(GL_TEXTURE_2D, this->layout.left_tex);

      x[1] = this->layout.x[0];
      y[1] = this->layout.height;

      glBegin( GL_QUADS );
      glTexCoord2d(0.0f, 0.0f); glVertex2d(x[0], y[0]);
      glTexCoord2d(1.0f, 0.0f); glVertex2d(x[1], y[0]);
      glTexCoord2d(1.0f, 1.0f); glVertex2d(x[1], y[1]);
      glTexCoord2d(0.0f, 1.0f); glVertex2d(x[0], y[1]);
      glEnd();

      glBindTexture(GL_TEXTURE_2D, this->layout.bot_tex);

      x[1] = this->layout.width;
      y[1] = this->layout.y[0];

      glBegin( GL_QUADS );
      glTexCoord2d(0.0f, 0.0f); glVertex2d(x[0], y[0]);
      glTexCoord2d(1.0f, 0.0f); glVertex2d(x[1], y[0]);
      glTexCoord2d(1.0f, 1.0f); glVertex2d(x[1], y[1]);
      glTexCoord2d(0.0f, 1.0f); glVertex2d(x[0], y[1]);
      glEnd();

      glDisable(GL_BLEND);
      glDisable(GL_TEXTURE_2D);
    }


    /* -------------------------------------------------------------------- */
    /* Public API                                                           */
    /* -------------------------------------------------------------------- */

    void
    QFosphorSurface::setFrequencyRange(const double center_freq,
                                       const double span)
    {
      freq_axis_build(&this->freq_axis, center_freq, span, 10);
      this->layout.dirty = true; // FIXME more fine grain refresh
    }

    void
    QFosphorSurface::setPalette(std::string name)
    {
      this->palette = name;
    }

    void
    QFosphorSurface::sendFrame(void *frame, int frame_len)
    {
      this->frame.data  = frame;
      this->frame.dirty = true;
      QMetaObject::invokeMethod(this, "updateGL");
    }


    /* -------------------------------------------------------------------- */
    /* Private helpers                                                      */
    /* -------------------------------------------------------------------- */

    void
    QFosphorSurface::refreshPowerAxis()
    {
      char buf[32];
      int i;

      /* Release previous texture */
      if (this->layout.left_tex)
        this->deleteTexture(this->layout.left_tex);

      /* Create a pixmap of right size */
      QPixmap pixmap(this->layout.x[0], this->layout.height);
      pixmap.fill(Qt::transparent);

      QPainter painter(&pixmap);
      painter.setPen(QColor(255, 255, 255, 255));

      QFont font("Monospace");
      font.setPixelSize(10);
      painter.setFont(font);

      /* Paint labels */
      for (i=0; i<11; i++)
      {
        int yv = (int)(this->layout.height - this->layout.y[0] - i * this->layout.y_div);

        snprintf(buf, sizeof(buf)-1, "%d", (i - 10) * 10);
        buf[sizeof(buf)-1] = 0;

        painter.drawText(
          0, yv - 10,
          this->layout.x[0] - 5, 20,
          Qt::AlignRight | Qt::AlignVCenter,
          buf
        );
      }

      /* Create texture */
      this->layout.left_tex = this->bindTexture(pixmap);
    }

    void
    QFosphorSurface::refreshFrequencyAxis()
    {
      char buf[32];
      int n_div, i;

      /* Release previous texture */
      if (this->layout.bot_tex)
        this->deleteTexture(this->layout.bot_tex);

      /* Create a pixmap of right size */
      QPixmap pixmap(this->layout.width, this->layout.y[0]);
      pixmap.fill(Qt::transparent);

      QPainter painter(&pixmap);
      painter.setPen(QColor(255, 255, 255, 255));

      QFont font("Monospace");
      font.setPixelSize(10);
      painter.setFont(font);

      /* Paint labels */
      n_div = 10;

      for (i=0; i<=n_div; i++)
      {
        int xv = (int)(this->layout.x[0] + i * this->layout.x_div);
        int xl, xw;
        int flags;

        freq_axis_render(&this->freq_axis, buf, i-(n_div>>1));
        buf[sizeof(buf)-1] = 0;

        if (i == 0) {
          xl = xv - 10;
          xw = 60;
          flags = Qt::AlignLeft | Qt::AlignVCenter;
        } else if (i == n_div) {
          xl = xv - 50;
          xw = 60;
          flags = Qt::AlignRight | Qt::AlignVCenter;
        } else {
          xl = xv - 30;
          xw = 60;
          flags = Qt::AlignHCenter | Qt::AlignVCenter;
        }

        painter.drawText(
          xl, 0,
          xw, (int)this->layout.y[0],
          flags,
          buf
        );
      }

      /* Create texture */
      this->layout.bot_tex = this->bindTexture(pixmap);
    }

    void
    QFosphorSurface::refreshLayout()
    {
      int rsvd_tlbr[2], rsvd, avail, div, over;

      /* Split the X space */
      rsvd_tlbr[0] = 40;
      rsvd_tlbr[1] = 20;

      rsvd  = rsvd_tlbr[0] + rsvd_tlbr[1];
      avail = this->layout.width - rsvd;
      div   = avail / 10;
      over  = avail - 10 * div;

      this->layout.x_div = (float)div;
      this->layout.x[0]  = (float)(rsvd_tlbr[0] + (over / 2));
      this->layout.x[1]  = this->layout.x[0] + 10.0f * div + 1.0f;

      /* Split the Y space */
      rsvd_tlbr[0] = 10;
      rsvd_tlbr[1] = 20;

      rsvd  = rsvd_tlbr[0] + rsvd_tlbr[1];
      avail = this->layout.height - rsvd;
      div   = avail / 10;
      over  = avail - 10 * div;

      this->layout.y_div = (float)div;
      this->layout.y[0]  = (float)(rsvd_tlbr[1] + over / 2);
      this->layout.y[1]  = this->layout.y[0] + 10.0f * div + 1.0f;

      /* Refresh axis */
      this->refreshPowerAxis();
      this->refreshFrequencyAxis();

      /* All refreshed now */
      this->layout.dirty = false;
    }

  } /* namespace ettus */
} /* namespace gr */

// vim: ts=2 sw=2 expandtab
