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
/*
 file:		parameters.h
 describtion:
   file for the classes QRL_ParametersManager
*/



#include "scope_data.h"

QPL_ScopeData::QPL_ScopeData(int ntr, float d,QString c_name, QStringList t_name)
        :ntraces(ntr),dt(d),name(c_name),traceNames(t_name)
{
		visible = false;
                saving = false;
		plotting=true;
                //Save_File_Pointer=NULL;
                saveScopeTime=false;
                scopeRefreshRate=30.;
                ScopeValues.resize(ntraces);
                ScopeIndex.resize(ntraces);
                ScopeTime.resize(MAX_SCOPE_DATA);
                for (int t=0; t<ntraces; t++){
                        ScopeValues[t].resize(MAX_SCOPE_DATA);
                        ScopeIndex[t]=0;
                }
                scopeDt=1./100.;
                //name=std::string(c_name);
                d2d = new QPL_Data2Disk(dt);
}


 QPL_ScopeData::~QPL_ScopeData()
 {
      //wait();
      delete d2d;
 }

// void QRL_ScopeData::run(){
// //exec();
// 
// }



    int QPL_ScopeData::setScopeDt(double d)
{
//int ret=-1;

	scopeDt=d;

	
 //ret= Get_Scope_Data_Thread[n].setDt(d);
return 1;
}


double QPL_ScopeData::getScopeDt()
{
double ret=-1;

 ret= scopeDt;
return ret;
}

int QPL_ScopeData::setScopeRefreshRate(double rr)
{
int ret=-1;
if (rr>0. && rr<50.){

        scopeRefreshRate=rr;

}
 //ret= Get_Meter_Data_Thread[n].setRefreshRate(rr);
return ret;
}

double QPL_ScopeData::getScopeRefreshRate()
{
        double ret=-1;

                ret=scopeRefreshRate;

        return ret;
}

void QPL_ScopeData::setScopeTime(double v){
      if (ScopeIndex[0]<MAX_SCOPE_DATA){
	ScopeTime[ScopeIndex[0]]=v;
      }
}

 QVector<double> QPL_ScopeData::getScopeTime(){
	
// QVector<float> ret;
// mutex.lock();
// ret.resize(ScopeValues.size());
 //ret= Get_Led_Data_Thread[n].getValue();

// 	for (int i=0;i<ScopeIndex[n][t];i++)
//   	 	ret[t].append(ScopeValues.at(n).at(t).at(i));
	 return ScopeTime.mid(0,ScopeIndex[0]);
  	// ScopeValues[n][t].clear();
	// ScopeIndex[n][0]=0;
   
// mutex.unlock();
	//return ret;
}


 void QPL_ScopeData::setScopeValue(double v, int t){

if ( ntraces>0){ 
// mutex.lock();
if (t<ScopeValues.size()){
	if (ScopeIndex[0]<MAX_SCOPE_DATA){
		ScopeValues[t][ScopeIndex[0]]=(v);
		if (t==(ntraces-1))
			ScopeIndex[0]++;
	} else {
		ScopeValues[t][0]=(v);
		if (t==(ntraces-1))
			ScopeIndex[0]=1;
	}
}
// mutex.unlock();
}

}

//  QVector<float> TargetThread::getScopeValue(int n, int t){
// 	
// 
// QVector<float> ret;
// if (n<ScopeValues.size()){
// if (t<ScopeValues.at(n).size()){
//  //ret= Get_Led_Data_Thread[n].getValue();
//    for (int i=0;i<ScopeIndex[n][t];i++)
//    	ret.append(ScopeValues.at(n).at(t).at(i));
//   // ScopeValues[n][t].clear();
//    ScopeIndex[n][t]=0;
// }
// }
// return ret;
// } 

 QVector< QVector<double> > QPL_ScopeData::getScopeValue(){
	
// printf("getScopeindex %d\n",ScopeIndex[0]);
QVector< QVector<double> > ret;
// mutex.lock();
ret.resize(ScopeValues.size());
 //ret= Get_Led_Data_Thread[n].getValue();

   for (int t=0; t<ScopeValues.size(); ++t){
// 	for (int i=0;i<ScopeIndex[n][t];i++)
//   	 	ret[t].append(ScopeValues.at(n).at(t).at(i));
	ret[t]=ScopeValues.at(t).mid(0,ScopeIndex[0]);
  	// ScopeValues[n][t].clear();
	// ScopeIndex[n][0]=0;
   }
ScopeIndex[0]=0;
// mutex.unlock();
	return ret;
}

//  QVector< QVector<float> > TargetThread::getScopeValue(int n){
// 	
// 
// //QVector< QVector<float> > ret;
// //ret.resize(ScopeValues.at(n).size());
//  //ret= Get_Led_Data_Thread[n].getValue();
// 
//    for (int t=0; t<ScopeValues.at(n).size(); ++t){
// //	for (int i=0;i<ScopeIndex[n][t];i++)
// //  	 	ret[t].append(ScopeValues.at(n).at(t).at(i));
//   	// ScopeValues[n][t].clear();
// 	 ScopeIndex[n][t]=0;
//    }
// 	//ret=ScopeValues[n];
// return ScopeValues[n];
// } 
 bool QPL_ScopeData::dataAvailable() {

    return ScopeValues.size()>0;
 }



// void  QPL_ScopeData::startSaving(FILE* save_file_pointer,double save_time){
//         Save_File_Pointer=save_file_pointer;
//         Save_Time=save_time;
//         saving=true;
// }
// FILE*  QPL_ScopeData::getSaveFilePtr() {
//
//         return Save_File_Pointer;
//
// }
//      void  QPL_ScopeData::stopSaving(){
//         saving=false;
//         fclose(Save_File_Pointer);
//         Save_File_Pointer=NULL;
//         //emit stopSaving(index);
//
// }
//       int  QPL_ScopeData::n_points_to_save(){
//         int n_points;
//
//         n_points = (int)(Save_Time/dt);
//         if (n_points < 0) return 0;
//         return n_points;
//
// }
//
//   void  QPL_ScopeData::set_points_counter(int cnt){
//
//   Saved_Points=cnt;
//
// }
//
