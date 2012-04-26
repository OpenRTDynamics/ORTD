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


#ifndef DATA2DISK_H
#define DATA2DISK_H

#include <QtCore>


class QPL_Data2Disk : public QObject
{
  Q_OBJECT
public:
    enum saveFormats {ascii};
    QPL_Data2Disk(float d);
   ~QPL_Data2Disk();
    int getIsSaving(){return Is_Target_Saving;}
    void writeNextData(double d,int a);
    void newLine();
   void setSaveTime(double t) {saveTime=t;}
   double getSaveTime(){return saveTime;}
    bool startSaving(const char * name, double save_time);
     bool startSaving();
     void stopSaving();
      int n_points_to_save();
    void set_points_counter(int cnt);
    void setDt(float d){dt=d;}
    //char name[MAX_NAMES_SIZE];
 int getSavedPoints(){return Saved_Points;}
    void setSaveScopeTime(bool b){saveScopeTime=b;}
    bool isSaveScopeTime(){return saveScopeTime;}
   void setFileName(QString str);
   QString getFileName() {return fileName;}
private:
        bool openSaveFile(const char * name);
        void closeSaveFile();
          FILE* save_file;
          unsigned int Is_Target_Saving;
          saveFormats saveFormat;
          QString fileName;
          double seconds;
          double saveTime;
          bool saveScopeTime;
          int Saved_Points;
          float dt;
};

#endif // DATA2DISK_H
