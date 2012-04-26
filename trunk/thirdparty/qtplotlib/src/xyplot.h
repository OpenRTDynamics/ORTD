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

#ifndef XYPLOT_H
#define XYPLOT_H

#include "qtplot_global.h"

#include "xyplot_trace.h"
#include <qwt_plot.h>

class  QPL_XYPlot : public QwtPlot
{
   Q_OBJECT

public:
   QPL_XYPlot(QWidget *parent = 0);
   virtual ~QPL_XYPlot();
   void initTraces(int ncurve, int nsoll);
//   void changeRefreshRate(double);
    void changeDataPoints(double);
//   void changeDivider(double);
//   unsigned int getDivider(){return Divider;}
//   void changeDX(double);
//   double getDt(){return dt;}
//   double getRefreshRate(){return RefreshRate;}
   int getDataPoints(){return NDataSoll;}
//   double getDX(){return dx;}
//   int getDXDistance(){return NDistance;}
//   void setGridColor(QColor);
//   QColor getGridColor(){return gridColor;}
//   void setBgColor(QColor);
//   QColor getBgColor(){return bgColor;}
 // QPL_XYPlotTrace* trace(int trace){if (trace<Scope->getNTraces()) return Traces[trace]; else return NULL;}
 QPL_XYPlotTrace* trace(int trace){return Traces[trace]; }

    void setValue(const QVector< QVector< QVector<float> > > &v);

//    void setVerbose(int v){Verbose=v;}
//    void setFileVersion(qint32 v){fileVersion=v;}
//    void setPlotting(bool b);
//    bool isPlotting(){return plotting;}
//    void setTraceName(int trace, const QString &text);
//   QString getTraceName(int trace){return Traces[trace]->getName();}
    int getNCurve(){return Ncurve;}
public slots:
   void refresh();
//protected slots:
//  void closeEvent ( QCloseEvent * event ){event->ignore(); this->hide(); }
private:
  qint32 fileVersion;
  int Verbose;
  bool plotting;
  double Value;
  double lastTime;
  //QPL_ScopeData *Scope;
  unsigned int NDataMax,Ncurve,NDataSoll, MaxDataPoints,Divider;
  int time,time2;
  double xmin,xmax,dx,dt;
  double xMajorTicks,xStep;
  int NDistance;
  QTimer *timer;

  double RefreshRate;

  QwtPlotGrid *grid;
  QColor gridColor;
  QColor bgColor;
  QwtPlotPicker *picker;
  QwtPlotMarker *zeroLine,*vertLine;
  QwtPlotMarker *bottomText;
  QwtPlotZoomer *zoomer[2];
  QwtPlotPanner *panner;
  int index;
  int style;
  int yMajorTicks;
  double yStep, yOffset, dy ,ymin,ymax;
   QPL_XYPlotTrace **Traces;

  friend QDataStream& operator<<(QDataStream &out, const QPL_XYPlot &d);
  friend QDataStream& operator>>(QDataStream &in, QPL_XYPlot(&d));
};
        QDataStream& operator<<(QDataStream &out, const QPL_XYPlot &d);
        QDataStream& operator>>(QDataStream &in, QPL_XYPlot(&d));





#endif // XYPLOT_H
