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

#ifndef XYPLOT_TRACE_H
#define XYPLOT_TRACE_H



#include <QtGui>

#include <qwt_plot.h>
#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qwt_symbol.h>
#include <qwt_scale_draw.h>
#include <qwt_math.h>
#include "qtplot_global.h"


class QPL_XYPlotTrace
{

 //friend class PlottingScopeDataThread;
public:
 QPL_XYPlotTrace(QwtPlot *parent, unsigned int maxdatapoints, int j);
~QPL_XYPlotTrace();
    void setFileVersion(qint32 v){fileVersion=v;}
  void changeNDataSoll(int ds);

   void setGridColor(const QColor& c){gridColor=c;}
   QColor getColor(){return  brush.color();}
   void setColor(const QColor&);
   void setWidth(int);
   int getWidth();
      void setDx(double);
   double getDx();
      void setDy(double);
   double getDy();
      void setXOffset(double);
   double getXOffset();
      void setYOffset(double);
   double getYOffset();
   void show(bool);
   bool isVisible(){return  visible;}
   //PlottingScopeDataThread* getThread(){return Plotting_Scope_Data_Thread;}

   void setName(const QString &text);
   QString getName(){return traceName;}
   void setValue(int index, double x, double y);
   void moveDataToLeft(int s);
   void moveDataToRight(int s);

   //saving is missing
       void setLineStyle(QwtPlotCurve::CurveStyle s){cData->setStyle(s); oldStyle=s;}
        QwtPlotCurve::CurveStyle    getLineStyle() {return cData->style();}
       void setSymbolPenColor(const QColor& c){sym.setPen(QColor(c));  cData->setSymbol(sym); }
       QColor getSymbolPenColor(){return sym.pen().color();}
       void setSymbolBrushColor(const QColor& c){sym.setBrush(QColor(c));  cData->setSymbol(sym); }
       QColor getSymbolBrushColor(){return sym.brush().color();}
       void setSymbolStyle(QwtSymbol::Style s){sym.setStyle(s); cData->setSymbol(sym);oldSymbol=s;}
       QwtSymbol::Style getSymbolStyle(){return sym.style();}
       void setSymbolSize(int s){sym.setSize(s);  cData->setSymbol(sym); }
       int getSymbolSize(){ return sym.size().width();}

       void setSymbol(QwtSymbol s){sym=s;cData->setSymbol(sym);  }
       QwtSymbol getSymbol(){return cData->symbol();}

private:
        qint32 fileVersion;
        QwtPlot *qwtPlot;
        QColor gridColor;
        QwtPlotCurve::CurveStyle oldStyle;
        QwtSymbol::Style oldSymbol;
        int index;
        double dx;
        double dy;
        int lineWidth;
        QBrush brush;
        QwtSymbol sym;
        QString traceName;
        bool visible;
        unsigned int  MaxDataPoints,NDataSoll,xmax;
        double xOffset,yOffset;
        double *d_x;
        double * d_y;
        double * d_yempty;
        QwtPlotCurve *cData;

        friend QDataStream& operator<<(QDataStream &out, const QPL_XYPlotTrace  &d);
         friend QDataStream& operator>>(QDataStream &in, QPL_XYPlotTrace &d);
         friend class QPL_XYPlot;
};
        QDataStream& operator<<(QDataStream &out, const QPL_XYPlotTrace &d);
        QDataStream& operator>>(QDataStream &in, QPL_XYPlotTrace(&d));


#endif // XYPLOT_TRACE_H
