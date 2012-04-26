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

#ifndef SCOPE_H
#define SCOPE_H

#include "qtplot_global.h"
#include <scope_trace.h>
#include <scope_data.h>

#include <qwt_plot.h>

class  QPL_Scope : public QwtPlot
{
   Q_OBJECT
 //friend class PlottingScopeDataThread;
public:
   enum PlottingMode {roll,overwrite,trigger,external_trigger,hold};
   //QPL_Scope(QWidget *parent = 0,QPL_ScopeData *scope=0,int ind=0);
   QPL_Scope(QWidget *parent = 0);
   virtual ~QPL_Scope();
   void initTraces(QPL_ScopeData *scope,int ind);
   void changeRefreshRate(double);
   void startRefresh(){   timer->start((int)(1./RefreshRate*1000.)); }
   void stopRefresh(){timer->stop(); }
   void changeDataPoints(double);
   void changeDivider(double);
   unsigned int getDivider(){return Divider;}
   void changeDX(double);
   double getDt(){return dt;}
   double getRefreshRate(){return RefreshRate;}
   int getDataPoints(){return NDataSoll;}
   double getDX(){return dx;}
   int getDXDistance(){return NDistance;}
   void setGridColor(QColor);
   QColor getGridColor(){return gridColor;}
   void setBgColor(QColor);
   QColor getBgColor(){return bgColor;}
   QPL_ScopeTrace* trace(int trace){if (trace<Scope->getNTraces()) return Traces[trace]; else return NULL;}
//   void setSaveTime(double t) {saveTime=t;}
//   double getSaveTime(){return saveTime;}
//   void setFileName(QString str) {fileName=str;}
//   QString getFileName() {return fileName;}
   void setPlottingMode(PlottingMode p);
   void setPlottingDirection(Qt::LayoutDirection d);
   int getPlottingMode() {return (int)plottingMode ;}
   Qt::LayoutDirection getPlottingDirection() {return direction;}
   void setTriggerUpDirection(bool b){triggerUp=b;}
   bool getTriggerUpDirection(){return triggerUp;}

   void setTriggerLevel(double l){triggerLevel=l;}
   double getTriggerLevel(){return triggerLevel;}
   void setTriggerChannel(int trace){ triggerChannel=trace;}
   int getTriggerChannel(){return  triggerChannel;}
   void setSingleMode(bool b){singleMode=b;}
    bool getSingleMode(){return singleMode;}
   void startSingleRun();
      void setZeroAxis(bool b,int);
   void setTraceLabel(bool b, int);
   void setUnitLabel(bool b, int);
   void setAverageLabel(bool b, int);
   void setMinLabel(bool b, int);
   void setMaxLabel(bool b, int);
   void setPPLabel(bool b, int);
   void setRMSLabel(bool b, int);
 bool getTraceLabel(int trace){return Traces[trace]->isLabelVisible(QPL_ScopeTrace::lt_trace);}
 bool getUnitLabel(int trace){return Traces[trace]->isLabelVisible(QPL_ScopeTrace::lt_unit);}
 bool getAverageLabel(int trace){return Traces[trace]->isLabelVisible(QPL_ScopeTrace::lt_average);}
 bool getMinLabel(int trace){return Traces[trace]->isLabelVisible(QPL_ScopeTrace::lt_min);}
 bool getMaxLabel(int trace){return Traces[trace]->isLabelVisible(QPL_ScopeTrace::lt_max);}
 bool getPPLabel(int trace){return Traces[trace]->isLabelVisible(QPL_ScopeTrace::lt_pp);}
 bool getRMSLabel(int trace){return Traces[trace]->isLabelVisible(QPL_ScopeTrace::lt_rms);}
   bool getZeroAxis(int trace){return Traces[trace]->zeroAxis.isVisible();}
    void setValue(const QVector< QVector<double> > &v);
    void setTime(const QVector<double> &t);
    void setVerbose(int v){Verbose=v;}
    void setFileVersion(qint32 v){fileVersion=v;}
    void setPlotting(bool b);
    bool isPlotting(){return plotting;}
    void setTraceName(int trace, const QString &text);
   QString getTraceName(int trace){return Traces[trace]->getName();}
   signals:
       void triggerEvent();
       void triggerWaitingEvent();

public slots:
   void manualTriggerSignal(){triggerSearch=false;}
   void refresh();
  //void setValue(int,float);
//protected slots:
//  void closeEvent ( QCloseEvent * event ){event->ignore(); this->hide(); }
private:
  qint32 fileVersion;
  int Verbose;
  bool plotting;
  double Value;
  double lastTime;
 QPL_ScopeData *Scope;
  unsigned int NDataMax,Ncurve,NDataSoll, MaxDataPoints,Divider;
  int time,time2;
  double xmin,xmax,dx,dt;
  double xMajorTicks,xStep;
  int NDistance;
  QTimer *timer;

  double RefreshRate;
//  double saveTime;
//  QString fileName;
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
  QPL_ScopeTrace **Traces;
  Qt::LayoutDirection direction;
  PlottingMode plottingMode;
  bool triggerSearch,triggerUp,singleMode,singleModeRunning;
  double triggerLevel,lastValue;int triggerChannel;
  //PlottingScopeDataThread* Plotting_Scope_Data_Thread;
  friend QDataStream& operator<<(QDataStream &out, const QPL_Scope &d);
  friend QDataStream& operator>>(QDataStream &in, QPL_Scope(&d));
};
        QDataStream& operator<<(QDataStream &out, const QPL_Scope &d);
        QDataStream& operator>>(QDataStream &in, QPL_Scope(&d));


#endif // SCOPE_H
