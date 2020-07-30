/* -*- c++ -*- */
/*
 * Copyright 2013-2015 Sylvain Munaut
 * Copyright 2015 Ettus Research
 * Copyright 2020 Ettus Research, A National Instruments Brand.
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

#include "QFosphorColorMapper.h"
#include "QFosphorSurface.h"

#include <stdio.h>
#include <string.h>

#include <QFont>
#include <QPainter>
#include <QPixmap>


namespace gr {

namespace ettus {

QFosphorSurface::QFosphorSurface(int fft_bins,
                                 int pwr_bins,
                                 int wf_lines,
                                 QWidget* parent)
    : QGLWidget(parent),
      fft_bins(fft_bins),
      pwr_bins(pwr_bins),
      wf_lines(wf_lines),
      grid_enabled(true),
      palette("iron")
{
    this->setFocusPolicy(Qt::StrongFocus);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    memset(&this->frame, 0, sizeof(this->frame));
    memset(&this->wf, 0, sizeof(this->wf));
    memset(&this->layout, 0, sizeof(this->layout));

    this->frame.vbo_buf = new float[4 * fft_bins]();
    this->wf.data_buf = new uint8_t[wf_lines * fft_bins]();

    this->setFrequencyRange(0.0, 0.0);
}

QFosphorSurface::~QFosphorSurface()
{
    delete[] this->frame.vbo_buf;
    delete[] this->wf.data_buf;
}


/* -------------------------------------------------------------------- */
/* Overloaded GL functions                                              */
/* -------------------------------------------------------------------- */

void QFosphorSurface::initializeGL()
{
    initializeGLFunctions();

    /* Init frame texture */
    glGenTextures(1, &this->frame.tex);

    glBindTexture(GL_TEXTURE_2D, this->frame.tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_R8,
                 this->fft_bins,
                 this->pwr_bins,
                 0,
                 GL_RED,
                 GL_UNSIGNED_BYTE,
                 NULL);

    /* Init frame VBO */
    glGenBuffers(1, &this->frame.vbo);

    glBindBuffer(GL_ARRAY_BUFFER, this->frame.vbo);

    glBufferData(
        GL_ARRAY_BUFFER, 2 * sizeof(float) * 2 * this->fft_bins, NULL, GL_DYNAMIC_DRAW);

    /* Init waterfall texture */
    glGenTextures(1, &this->wf.tex);

    glBindTexture(GL_TEXTURE_2D, this->wf.tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_R8,
                 this->fft_bins,
                 this->wf_lines,
                 0,
                 GL_RED,
                 GL_UNSIGNED_BYTE,
                 NULL);

    /* Color map */
    this->cmap = new QFosphorColorMapper(this);
}

void QFosphorSurface::resizeGL(int width, int height)
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
    this->layout.width = width;
    this->layout.height = height;
    this->layout.dirty = true;
}

void QFosphorSurface::paintGL()
{
    /* If no data, abort early */
    if (!this->frame.data)
        return;

    /* Upload texture if needed */
    if (this->frame.dirty) {
        this->uploadFrameData();
    }

    if (this->wf.dirty) {
        this->uploadWaterfallData();
    }

    /* Refresh layout if needed */
    if (this->layout.dirty) {
        this->refreshLayout();
    }

    /* Clear everything */
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Draw the various UI element */
    this->drawHistogram();
    this->drawHistogramIntensityScale();
    this->drawSpectrum();
    this->drawGrid();

    if (this->layout.wf_enabled) {
        this->drawWaterfall();
        this->drawWaterfallIntensityScale();
    }

    this->drawMargins();
}


/* -------------------------------------------------------------------- */
/* Public API                                                           */
/* -------------------------------------------------------------------- */

void QFosphorSurface::setFrequencyRange(const double center_freq, const double span)
{
    freq_axis_build(&this->freq_axis, center_freq, span, 10);
    this->layout.dirty = true; // FIXME more fine grain refresh
}

void QFosphorSurface::setWaterfall(bool enabled)
{
    this->layout.wf_enabled = enabled;
    this->layout.dirty = true;
}

void QFosphorSurface::setGrid(bool enabled) { this->grid_enabled = enabled; }

void QFosphorSurface::setPalette(std::string name) { this->palette = name; }

void QFosphorSurface::sendFrame(void* frame, int frame_len)
{
    this->frame.data = frame;
    this->frame.dirty = true;
    QMetaObject::invokeMethod(this, "updateGL");
}

void QFosphorSurface::sendWaterfall(const uint8_t* wf, int n)
{
    int m;

    // Limit to max height
    if (n > this->wf_lines) {
        wf += this->fft_bins * (n - this->wf_lines);
        n = this->wf_lines;
    }

    // Copy the new data
    while (n) {
        m = n > (this->wf_lines - this->wf.pos) ? (this->wf_lines - this->wf.pos) : n;
        memcpy(&this->wf.data_buf[this->wf.pos * this->fft_bins], wf, m * this->fft_bins);
        this->wf.pos = (this->wf.pos + m) % this->wf_lines;
        wf += m * this->fft_bins;
        n -= m;
    }

    // Force refresh
    this->wf.dirty = true;
}


/* -------------------------------------------------------------------- */
/* Private helpers                                                      */
/* -------------------------------------------------------------------- */

void QFosphorSurface::drawHistogram()
{
    float x[2], y[2];
    float e = 0.5f / this->fft_bins;

    /* Draw Histogram texture */
    this->cmap->enable(this->palette, this->frame.tex);

    x[0] = this->layout.x[1];
    x[1] = this->layout.x[2];
    y[0] = this->layout.y[3];
    y[1] = this->layout.y[4];

    glBegin(GL_QUADS);
    glTexCoord2d(0.0f + e, -0.038f);
    glVertex2d(x[0], y[0]);
    glTexCoord2d(1.0f + e, -0.038f);
    glVertex2d(x[1], y[0]);
    glTexCoord2d(1.0f + e, 1.000f);
    glVertex2d(x[1], y[1]);
    glTexCoord2d(0.0f + e, 1.000f);
    glVertex2d(x[0], y[1]);
    glEnd();

    this->cmap->disable();
}

void QFosphorSurface::drawHistogramIntensityScale()
{
    this->cmap->drawScale(this->palette,
                          this->layout.x[2] + 2.0f,
                          this->layout.y[3],
                          this->layout.x[2] + 10.0f,
                          this->layout.y[4]);
}

void QFosphorSurface::drawSpectrum()
{
    /* Setup the 2D transform */
    glPushMatrix();

    glTranslatef(this->layout.x[1], this->layout.y[3], 0.0f);

    glScalef(this->layout.x[2] - this->layout.x[1],
             this->layout.y[4] - this->layout.y[3],
             1.0f);

    glScalef(1.0f / this->fft_bins, 0.9632f / 256.0f, 1.0f);

    glTranslatef(0.0f,
                 9.4208f, /* (100 - 96.32) / 100 * 256 */
                 0.0f);

    /* Setup GL state */
    glBindBuffer(GL_ARRAY_BUFFER, this->frame.vbo);
    glVertexPointer(2, GL_FLOAT, 0, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);

    /* Draw Max-Hold */
    glColor4f(1.0f, 0.0f, 0.0f, 0.75f);

    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_STRIP, 0, this->fft_bins);
    glDisableClientState(GL_VERTEX_ARRAY);

    /* Draw Live */
    glColor4f(1.0f, 1.0f, 1.0f, 0.75f);

    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_STRIP, this->fft_bins, this->fft_bins);
    glDisableClientState(GL_VERTEX_ARRAY);

    /* Cleanup GL state */
    glDisable(GL_BLEND);

    /* Restore */
    glPopMatrix();
}

void QFosphorSurface::drawGrid()
{
    float x[2], y[2];
    int i;

    if (!this->grid_enabled)
        return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_LINES);
    glColor4f(0.02f, 0.02f, 0.02f, 0.5f);

    /* Vertical div */
    x[0] = this->layout.x[1];
    x[1] = this->layout.x[2];

    for (i = 0; i < 11; i++) {
        y[0] = this->layout.y[3] + i * this->layout.y_div + 0.5f;

        glVertex2f(x[0], y[0]);
        glVertex2f(x[1], y[0]);
    }

    /* Horizontal div */
    y[0] = this->layout.y[3];
    y[1] = this->layout.y[4];

    for (i = 0; i < 11; i++) {
        x[0] = this->layout.x[1] + i * this->layout.x_div + 0.5f;

        glVertex2f(x[0], y[0]);
        glVertex2f(x[0], y[1]);
    }

    glEnd();
    glDisable(GL_BLEND);
}

void QFosphorSurface::drawWaterfall()
{
    float x[2], y[2];
    float e = 0.5f / this->fft_bins;
    float v[2];

    /* Draw Waterfall texture */
    this->cmap->enable(this->palette, this->wf.tex);

    x[0] = this->layout.x[1];
    x[1] = this->layout.x[2];
    y[0] = this->layout.y[1];
    y[1] = this->layout.y[2];

    v[0] = (float)(this->wf.pos) / (float)(this->wf_lines);
    v[1] = v[0] + 1.0f;

    glBegin(GL_QUADS);
    glTexCoord2d(0.0f + e, v[0]);
    glVertex2d(x[0], y[0]);
    glTexCoord2d(1.0f + e, v[0]);
    glVertex2d(x[1], y[0]);
    glTexCoord2d(1.0f + e, v[1]);
    glVertex2d(x[1], y[1]);
    glTexCoord2d(0.0f + e, v[1]);
    glVertex2d(x[0], y[1]);
    glEnd();

    this->cmap->disable();
}

void QFosphorSurface::drawWaterfallIntensityScale()
{
    this->cmap->drawScale(this->palette,
                          this->layout.x[2] + 2.0f,
                          this->layout.y[1],
                          this->layout.x[2] + 10.0f,
                          this->layout.y[2]);
}

void QFosphorSurface::drawMargins()
{
    float x[2], y[2];

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    x[0] = this->layout.x[0];
    y[0] = this->layout.y[2];

    glBindTexture(GL_TEXTURE_2D, this->layout.pwr_tex);

    x[1] = this->layout.x[1];
    y[1] = this->layout.y[5];

    glBegin(GL_QUADS);
    glTexCoord2d(0.0f, 0.0f);
    glVertex2d(x[0], y[0]);
    glTexCoord2d(1.0f, 0.0f);
    glVertex2d(x[1], y[0]);
    glTexCoord2d(1.0f, 1.0f);
    glVertex2d(x[1], y[1]);
    glTexCoord2d(0.0f, 1.0f);
    glVertex2d(x[0], y[1]);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, this->layout.freq_tex);

    x[1] = this->layout.x[3];
    y[1] = this->layout.y[3];

    glBegin(GL_QUADS);
    glTexCoord2d(0.0f, 0.0f);
    glVertex2d(x[0], y[0]);
    glTexCoord2d(1.0f, 0.0f);
    glVertex2d(x[1], y[0]);
    glTexCoord2d(1.0f, 1.0f);
    glVertex2d(x[1], y[1]);
    glTexCoord2d(0.0f, 1.0f);
    glVertex2d(x[0], y[1]);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void QFosphorSurface::uploadFrameData()
{
    int i;

    /* Upload new texture */
    glBindTexture(GL_TEXTURE_2D, this->frame.tex);

    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    this->fft_bins,
                    this->pwr_bins,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    this->frame.data);

    /* Compute the new VBO data */
    for (i = 0; i < this->fft_bins; i++) {
        uint8_t* data = (uint8_t*)this->frame.data + (this->fft_bins * this->pwr_bins);
        int maxh_idx = 2 * i;
        int live_idx = 2 * (i + this->fft_bins);

        this->frame.vbo_buf[maxh_idx] = i;
        this->frame.vbo_buf[maxh_idx + 1] = data[i];

        this->frame.vbo_buf[live_idx] = i;
        this->frame.vbo_buf[live_idx + 1] = data[i + this->fft_bins];
    }

    /* Upload new VBO data */
    glBindBuffer(GL_ARRAY_BUFFER, this->frame.vbo);

    glBufferData(GL_ARRAY_BUFFER,
                 2 * sizeof(float) * 2 * this->fft_bins,
                 this->frame.vbo_buf,
                 GL_DYNAMIC_DRAW);

    /* Data is fresh */
    this->frame.dirty = false;
}

void QFosphorSurface::uploadWaterfallData()
{
    glBindTexture(GL_TEXTURE_2D, this->wf.tex);

    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    this->fft_bins,
                    this->wf_lines,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    this->wf.data_buf);
}

void QFosphorSurface::refreshPowerAxis()
{
    char buf[32];
    int i;

    /* Release previous texture */
    if (this->layout.pwr_tex)
        this->deleteTexture(this->layout.pwr_tex);

    /* Create a pixmap of right size */
    QPixmap pixmap((int)(this->layout.x[1] - this->layout.x[0]),
                   (int)(this->layout.y[5] - this->layout.y[2]));
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setPen(QColor(255, 255, 255, 255));

    QFont font("Monospace");
    font.setPixelSize(10);
    painter.setFont(font);

    /* Paint labels */
    for (i = 0; i < 11; i++) {
        int yv = (int)(this->layout.y[5] - this->layout.y[3] - i * this->layout.y_div);

        snprintf(buf, sizeof(buf) - 1, "%d", (i - 10) * 10);
        buf[sizeof(buf) - 1] = 0;

        painter.drawText(0,
                         yv - 10,
                         this->layout.x[1] - this->layout.x[0] - 5,
                         20,
                         Qt::AlignRight | Qt::AlignVCenter,
                         buf);
    }

    /* Create texture */
    this->layout.pwr_tex = this->bindTexture(pixmap);
}

void QFosphorSurface::refreshFrequencyAxis()
{
    char buf[32];
    int n_div, i;

    /* Release previous texture */
    if (this->layout.freq_tex)
        this->deleteTexture(this->layout.freq_tex);

    /* Create a pixmap of right size */
    QPixmap pixmap((int)(this->layout.x[3] - this->layout.x[0]),
                   (int)(this->layout.y[3] - this->layout.y[2]));
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setPen(QColor(255, 255, 255, 255));

    QFont font("Monospace");
    font.setPixelSize(10);
    painter.setFont(font);

    /* Paint labels */
    n_div = 10;

    for (i = 0; i <= n_div; i++) {
        int xv = (int)(this->layout.x[1] - this->layout.x[0] + i * this->layout.x_div);
        int xl, xw;
        int flags;

        freq_axis_render(&this->freq_axis, buf, i - (n_div >> 1));
        buf[sizeof(buf) - 1] = 0;

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
            xl, 0, xw, (int)(this->layout.y[3] - this->layout.y[2]), flags, buf);
    }

    /* Create texture */
    this->layout.freq_tex = this->bindTexture(pixmap);
}

void QFosphorSurface::refreshLayout()
{
    int rsvd_pos[3], rsvd, avail, n_div, div, over, over_pos[3];

    /* Split the X space */
    rsvd_pos[0] = 40;
    rsvd_pos[1] = 20;

    rsvd = rsvd_pos[0] + rsvd_pos[1];
    avail = this->layout.width - rsvd - 1;
    div = avail / 10;
    over = avail - 10 * div;

    over_pos[0] = over / 2;
    over_pos[1] = over - over_pos[0];

    this->layout.x_div = (float)div;
    this->layout.x[0] = 0.0f;
    this->layout.x[1] = (float)(rsvd_pos[0] + over_pos[0]);
    this->layout.x[2] = this->layout.x[1] + 10.0f * div + 1.0f;
    this->layout.x[3] = this->layout.width;

    /* Split the Y space */
    rsvd_pos[0] = this->layout.wf_enabled ? 10 : 0;
    rsvd_pos[1] = 20;
    rsvd_pos[2] = 10;

    rsvd = rsvd_pos[0] + rsvd_pos[1] + rsvd_pos[2];
    avail = this->layout.height - rsvd - (this->layout.wf_enabled ? 2 : 1);
    n_div = this->layout.wf_enabled ? 20 : 10;
    div = avail / n_div;
    over = avail - n_div * div;

    if (this->layout.wf_enabled) {
        over_pos[0] = over / 3;
        over_pos[1] = over - (2 * over_pos[0]);
        over_pos[2] = over_pos[0];
    } else {
        over_pos[0] = 0;
        over_pos[1] = over / 2;
        over_pos[2] = over - over_pos[1];
    }

    this->layout.y_div = (float)div;

    this->layout.y[0] = 0.0f;

    if (this->layout.wf_enabled) {
        this->layout.y[1] = this->layout.y[0] + (float)(rsvd_pos[0] + over_pos[0]);
        this->layout.y[2] = this->layout.y[1] + 10.0f * div + 1.0f;
    } else {
        this->layout.y[1] = 0.0f;
        this->layout.y[2] = 0.0f;
    }

    this->layout.y[3] = this->layout.y[2] + (float)(rsvd_pos[1] + over_pos[1]);
    this->layout.y[4] = this->layout.y[3] + 10.0f * div + 1.0f;

    this->layout.y[5] = this->layout.height;

    /* Refresh axis */
    this->refreshPowerAxis();
    this->refreshFrequencyAxis();

    /* All refreshed now */
    this->layout.dirty = false;
}

} /* namespace ettus */

} /* namespace gr */

// vim: ts=2 sw=2 expandtab
