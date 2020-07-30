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

#ifndef INCLUDED_ETTUS_QFOSPHORCOLORMAPPER_H
#define INCLUDED_ETTUS_QFOSPHORCOLORMAPPER_H

#include <QFile>
#include <QGLShaderProgram>
#include <QGradient>
#include <QObject>
#include <QPixmap>

#include <map>
#include <string>

namespace gr {

namespace ettus {

class QFosphorColorMapper : public ::QObject
{
    Q_OBJECT

public:
    QFosphorColorMapper(QObject* parent = NULL);

    int loadFromFile(QFile& file);
    bool addPalette(std::string name, QLinearGradient& gradient);
    bool addPalette(std::string name, QPixmap& pixmap);

    void drawScale(std::string name, float x0, float y0, float x1, float y1);

    void enable(std::string name, GLuint tex_id);
    void disable();

private:
    QGLShaderProgram* shader;

    int u_cmap;
    int u_tex;
    int u_range;

    std::map<std::string, GLuint> palettes;
};

} // namespace ettus

} // namespace gr

#endif /* INCLUDED_ETTUS_QFOSPHORCOLORMAPPER_H */

// vim: ts=2 sw=2 expandtab
