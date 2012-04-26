/***************************************************************************
 *   Copyright (C) 2010 by Holger Nahrstaedt                               *
 *                                                                         *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License           *
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

/*
 file:		scopes_trace.h
 describtion:
   file for the classes QRL_ScopeTrace
*/
#ifndef _SCOPE_TRACE_H
#define _SCOPE_TRACE_H 1

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



/**
 * @brief Display Trace
 */

class QPL_ScopeTrace
{
   
 //friend class PlottingScopeDataThread;
public:
  enum labelTypes{lt_trace,lt_unit,lt_average,lt_min,lt_max,lt_pp,lt_rms};
  QPL_ScopeTrace(QwtPlot *parent, unsigned int maxdatapoints, int j);
~QPL_ScopeTrace();
    void setFileVersion(qint32 v){fileVersion=v;}
    void refresh();
  void changeNDataSoll(int ds, double dt);
void setLabelsXValue(double x);
    void resetTime(){time=0;}	
   void setTime(int t){time=t;}
  int getTime() {return time;}
//   double* getPointerd_y(){return d_y;}
   void setGridColor(const QColor& c){gridColor=c;}
   QColor getColor(){return  brush.color();}
   void setColor(const QColor&);
   void setWidth(int);
   int getWidth();
   void setOffset(double);
   double getOffset();
   double getAverage();
   double getPP();
   void setDy(double);
   double getDy();
   void show(bool);
   bool isVisible(){return  visible;}
   void setZeroAxis(bool b);
   void showLabel(labelTypes lt);
   void hideLabel(labelTypes lt);
   bool isLabelVisible(labelTypes lt);
   void setLabelVisible(labelTypes lt, bool setvisible);
   bool getZeroAxis(){return zeroAxis.isVisible();}
  void setLabelCounter(int counter){labelCounter=counter;}
  int getLabelCounter(){return labelCounter;}
   //PlottingScopeDataThread* getThread(){return Plotting_Scope_Data_Thread;}
void setZeroAxis();
   void setName(const QString &text);
   QString getName(){return traceName;}
   void setValue(int i, double v);
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
       double* getY(){return d_y;}
       int getIndex(){return index;}

private:
        qint32 fileVersion;
	QwtPlot *qwtPlot;
	QColor gridColor;
        QwtPlotCurve::CurveStyle oldStyle;
        QwtSymbol::Style oldSymbol;
 	int index;
	double offset;
	double dy;
	int lineWidth;
	QBrush brush;
        QwtSymbol sym;
  	QwtPlotMarker zeroAxis;
	double average;
	double min,max;
	double PP,RMS;
	QwtPlotMarker traceLabel;
	QwtPlotMarker unitLabel;
	QwtPlotMarker averageLabel;
	QwtPlotMarker minLabel;
	QwtPlotMarker maxLabel;
	QwtPlotMarker ppLabel;
	QwtPlotMarker rmsLabel;
        double labelOffset;
	QString traceName;
	int labelCounter;
	bool visible;
//data
  unsigned int  MaxDataPoints,NDataSoll,xmax;
  double *d_x; 
	int dt;
	double * d_y;
	double * d_yempty;
	int time;
	int yt;

//curve
  QwtPlotCurve *cData;
  //PlottingScopeDataThread* Plotting_Scope_Data_Thread;
//    friend QDataStream& operator<<(QDataStream &out, const QRL_ScopeTrace &d);
//    friend QDataStream& operator>>(QDataStream &in, QRL_ScopeTrace(&d));
   friend QDataStream& operator<<(QDataStream &out, const QPL_ScopeTrace* d);
   friend QDataStream& operator>>(QDataStream &in, QPL_ScopeTrace* d);
  friend class QPL_Scope;
};
        QDataStream& operator<<(QDataStream &out, const QPL_ScopeTrace *d);
        QDataStream& operator>>(QDataStream &in, QPL_ScopeTrace(*d));


#endif
