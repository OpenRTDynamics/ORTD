/***************************************************************************
 *   Copyright (C) 2010 by Holger Nahrstaedt                               *
 *                                                                         *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License                  *
 *   as published by  the Free Software Foundation; either version 2       *
 *   of the License, or  (at your option) any later version.               *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef QTPLOT_GLOBAL_H
#define QTPLOT_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QtCore>
//#include <QtGui>

//#include <qwt_plot.h>
//#include <qwt_painter.h>
//#include <qwt_plot_canvas.h>
//#include <qwt_plot_marker.h>
//#include <qwt_plot_curve.h>
//#include <qwt_plot_grid.h>
//#include <qwt_plot_picker.h>
//#include <qwt_plot_zoomer.h>
//#include <qwt_plot_panner.h>
//#include <qwt_scale_widget.h>
//#include <qwt_legend.h>
//#include <qwt_scale_draw.h>
//#include <qwt_math.h>

#define MAX_SCOPE_DATA		10000


#if defined(QTPLOT_LIBRARY)
#  define QTPLOTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QTPLOTSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QTPLOT_GLOBAL_H
