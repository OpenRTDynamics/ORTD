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

#ifndef MATRIX_PLOT_H
#define MATRIX_PLOT_H

#include "qtplot_global.h"
#include <QtGui>
#include "matrix_view.h"

class  QPL_MatrixPlot : public QTableView
{
   Q_OBJECT

public:
     enum matrixDelegate {pixel,blackwhite,colorbar,text};
   QPL_MatrixPlot(QWidget *parent = 0);
   virtual ~QPL_MatrixPlot();
   void setValue(const QVector< QVector<float> > &v);
     void setMinScale(double min);
     void setMaxScale(double max);
     double getMinScale(){return minScale;}
     double getMaxScale(){return maxScale;}
     void setDelegate(matrixDelegate d);
     matrixDelegate getDelegate(){return actualDelegate;}
     void setPixelSize(int psize);
     int  getPixelSize(){return pixelSize;}
     void  setShowItemNumber(bool n);
     bool  getShowItemNumber(){return showItemNumber;}
   private:
     MatrixModel *model;
      PixelDelegate *pixelView;
       BlackWhiteDelegate *blackwhiteView;
       ColorBarDelegate *colorView;
       matrixDelegate actualDelegate;
      double minScale;
     double maxScale;
      int pixelSize;
     bool showItemNumber;

   friend QDataStream& operator<<(QDataStream &out, const QPL_MatrixPlot &d);
  friend QDataStream& operator>>(QDataStream &in, QPL_MatrixPlot(&d));
};
        QDataStream& operator<<(QDataStream &out, const QPL_MatrixPlot &d);
        QDataStream& operator>>(QDataStream &in, QPL_MatrixPlot(&d));





#endif // MATRIX_PLOT_H
