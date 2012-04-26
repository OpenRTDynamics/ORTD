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
 file:		parameters.h
 describtion:
   file for the classes QRL_ParametersManager
*/

#ifndef _SCOPE_DATA_H
#define _SCOPE_DATA_H 1

#include <QtCore>

#include "qtplot_global.h"
#include "data2disk.h"


class QPL_ScopeData //: public QThread
{
 // Q_OBJECT
public:
  QPL_ScopeData(int ntraces, float dt,QString, QStringList t_name);
  ~QPL_ScopeData();
  int getNTraces(){return ntraces;}
//  void setNTraces(int t){ntraces=t;}
  float getDt(){return dt;} 
//  void setDt(float d){dt=d;}
//  bool getIsSaving(){return saving;}
//  int getSavedPoints(){return Saved_Points;}
  QString getName(){return name;}

  int setScopeDt(double);
    double getScopeDt();
    int setScopeRefreshRate(double rr);
    double getScopeRefreshRate();
    void setScopeValue(double v, int t);
    void setScopeTime(double v);
     QVector<double> getScopeTime();
    QVector<double> getScopeValue(int t);
    QVector< QVector<double> > getScopeValue();
    bool dataAvailable();
    QPL_Data2Disk* data2disk(){return d2d;}
//    int start_saving_scope();
//    void startSaving(FILE* Save_File_Pointer,double Save_Time);
//     FILE* getSaveFilePtr();
//     void stopSaving();
//      int n_points_to_save();
//    void set_points_counter(int cnt);

    //char name[MAX_NAMES_SIZE];

    void setSaveScopeTime(bool b){saveScopeTime=b;}
    bool isSaveScopeTime(){return saveScopeTime;}
    
    void setPlotting(bool b){plotting=b;}
    bool isPlotting(){return plotting;}
    QStringList getTraceNames(){return traceNames;}
// 	int ntraces;
// protected:
//    void run();
private:
    bool saveScopeTime;
    const int ntraces;
    const  float dt;
    QString name;
    QStringList traceNames;
    bool saving;
    bool plotting;
    int visible;
//    FILE* Save_File_Pointer;
//    double Save_Time;
//    int Saved_Points;
    double scopeRefreshRate;
//     QRL_ScopeTrace **Traces;
    QVector <int>  ScopeIndex;
    QVector< QVector <double> >  ScopeValues;
    QVector <double> ScopeTime;
    QVector <int>  ScopeIndexOut;
    QVector< QVector <double> >  ScopeValuesOut;
    double scopeDt;
       QPL_Data2Disk* d2d;
};



#endif
