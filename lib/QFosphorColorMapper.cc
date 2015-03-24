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

#include "QFosphorColorMapper.h"

#include <QGLContext>

namespace gr {
  namespace ettus {

    QFosphorColorMapper::QFosphorColorMapper(QObject *parent)
      : QObject(parent)
    {
      /* Shader init */
      this->shader = new QGLShaderProgram(this);

      this->shader->addShaderFromSourceCode(QGLShader::Fragment,
        "uniform sampler2D cmap;\n"
        "uniform sampler2D tex;\n"
        "uniform vec2 range;\n"
        "\n"
        "void main()\n"
        "{\n"
        " float intensity = texture2D(tex, gl_TexCoord[0].st).x;\n"
        " float map = (intensity + range.y) * range.x;\n"
        " vec4 color = texture2D(cmap, vec2(map, 0.0));\n"
        " gl_FragColor = color;\n"
        "}\n"
      );
      this->shader->link();

      this->u_cmap  = this->shader->uniformLocation("cmap");
      this->u_tex   = this->shader->uniformLocation("tex");
      this->u_range = this->shader->uniformLocation("range");

      /* Load default set */
      QFile f(":/fosphor/palettes.txt");
      this->loadFromFile(f);
    }

    int
    QFosphorColorMapper::loadFromFile(QFile &file)
    {
      QLinearGradient *gradient = NULL;
      char *name;
      int kp = -1;

      /* Make sure it's open */
      if (!file.isOpen())
        file.open(QFile::ReadOnly);

      /* Scan until the end */
      while (!file.atEnd())
      {
        /* Grab a line */
        QByteArray line = file.readLine().simplified();

        /* Skip comments and empty lines */
        if (line.isEmpty() || line.startsWith('#'))
          continue;

        /* Are we in a palette ? */
        if (kp < 0)
        {
          sscanf(line.constData(), "%d %ms", &kp, &name);

          if (kp)
            gradient = new QLinearGradient();
        }
        else if (kp == 0)
        {
          /* Create palette */
          QString filename = line;
          QPixmap pixmap(filename);

          this->addPalette(name, pixmap);

          /* Release name & prepare for next */
          free(name);
          name = NULL;

          kp = -1;
        }
        else
        {
          float f[4];

          /* Read data point */
          sscanf(line.constData(), "%f %f %f %f", f, f+1, f+2, f+3);
          gradient->setColorAt(f[0], QColor::fromRgbF(f[1], f[2], f[3]));

          /* Is it over ? */
          if (!--kp)
          {
            /* Add the newly created palette */
            this->addPalette(name, *gradient);

            /* Release gradient, name & prepare for next */
            delete gradient;
            gradient = NULL;

            free(name);
            name = NULL;

            kp = -1;
          }
        }
      }

      return 0;
    }

    bool
    QFosphorColorMapper::addPalette(std::string name, QLinearGradient &gradient)
    {
      /* Configure the gradient */
      gradient.setStart(0,0);
      gradient.setFinalStop(1,0);
      gradient.setCoordinateMode(QGradient::ObjectBoundingMode);

      /* Draw it on a pixmap */
      QPixmap pixmap(256, 1);
      pixmap.fill(Qt::transparent);

      QPainter painter(&pixmap);
      painter.fillRect(0, 0, 256, 1, gradient);

      /* Create it from pixmap */
      return this->addPalette(name, pixmap);
    }

    bool
    QFosphorColorMapper::addPalette(std::string name, QPixmap &pixmap)
    {
      /* Convert to an OpenGL texture */
        /* Note: We use TEXTURE_2D because 1D isn't really supported by Qt
         * and it's also not in OpenGL ES */
      QGLContext *ctx = (QGLContext*) QGLContext::currentContext();
      GLuint tex_id = ctx->bindTexture(pixmap, GL_TEXTURE_2D);

      /* Configure behavior */
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      /* Insert new texture (remove old one if needed) */
      if (this->palettes.count(name))
        ctx->deleteTexture(this->palettes[name]);

      this->palettes[name] = tex_id;

      return true;
    }

    void
    QFosphorColorMapper::drawScale(std::string name, float x0, float y0, float x1, float y1)
    {
      /* Enable texture-2D */
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, this->palettes[name]);
      glEnable(GL_TEXTURE_2D);

      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

      /* Draw QUAD */
      glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f); glVertex2f(x0, y0);
      glTexCoord2f(0.0f, 0.0f); glVertex2f(x1, y0);
      glTexCoord2f(1.0f, 0.0f); glVertex2f(x1, y1);
      glTexCoord2f(1.0f, 0.0f); glVertex2f(x0, y1);
      glEnd();

      /* Disable texturing */
      glDisable(GL_TEXTURE_2D);
    }

    void
    QFosphorColorMapper::enable(std::string name, GLuint tex_id)
    {
      /* Enable shader */
      this->shader->bind();

      /* Texture unit 0: Main texture */
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex_id);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      this->shader->setUniformValue(this->u_tex, 0);

      /* Texture unit 1: Palette texture */
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, this->palettes[name]);
      this->shader->setUniformValue(this->u_cmap, 1);

      /* Range definition */
      this->shader->setUniformValue(this->u_range, 1.0f, 0.00f);
    }

    void
    QFosphorColorMapper::disable()
    {
      glActiveTexture(GL_TEXTURE0);
      this->shader->release();
    }

  } // namespace fosphor
} // namespace fosphor

// vim: ts=2 sw=2 expandtab
