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

#include "data2disk.h"



QPL_Data2Disk::QPL_Data2Disk(float d)
      :  dt(d)
    {

    Is_Target_Saving=0;
     saveFormat=ascii;
     seconds=0;
     save_file=NULL;
     saveScopeTime=false;
     fileName=tr("data");
}

QPL_Data2Disk::~QPL_Data2Disk(){


}

bool QPL_Data2Disk::startSaving(){

        bool ret;
    ret=openSaveFile(fileName.toLocal8Bit().data());
    if (ret){
        return true;
    } else
        return false;



}

   void QPL_Data2Disk::setFileName(QString str) {
        fileName=str;
 if (!fileName.contains(QDir::separator())){
     fileName=tr(".")+QDir::separator()+fileName;
 }

   }
bool QPL_Data2Disk::startSaving(const char * name, double save_time){
    bool ret;
    ret=openSaveFile(name);
    if (ret){
        fileName=QString(name);
        saveTime=save_time;
        return true;
    } else
        return false;
}

 bool QPL_Data2Disk::openSaveFile(const char * name){

 if (saveFormat==ascii){
  if ((save_file = fopen(name, "w+")) == NULL) {
        Is_Target_Saving=0;
      return false;

  } else {
    Is_Target_Saving=1;
    fileName=QString(name);
    return true;
  }
} else
    return false;

 }

  void QPL_Data2Disk::closeSaveFile(){

    fclose(save_file);
    save_file=NULL;
  }

   void QPL_Data2Disk::stopSaving(){
      closeSaveFile();
      Is_Target_Saving=0;
   }

   void QPL_Data2Disk::writeNextData(double d,int a){
    switch(a){
       case 5:
        fprintf(save_file, "%1.5f ", d);
        break;
        case 10:
        default:
         fprintf(save_file, "%1.10f ", d);

    }

   }
    void QPL_Data2Disk::newLine(){

        fprintf(save_file, "\n");

    }


      int  QPL_Data2Disk::n_points_to_save(){
        int n_points;

        n_points = (int)(saveTime/dt);
        if (n_points < 0) return 0;
        return n_points;

}

  void  QPL_Data2Disk::set_points_counter(int cnt){

  Saved_Points=cnt;

}

