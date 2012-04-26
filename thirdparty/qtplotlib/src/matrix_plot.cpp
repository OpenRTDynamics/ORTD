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

#include "matrix_plot.h"

QPL_MatrixPlot::QPL_MatrixPlot(QWidget *parent)
        :QTableView(parent)
{

     model = new MatrixModel(this);

     this->setShowGrid(false);
     this->horizontalHeader()->hide();
     this->verticalHeader()->hide();
     this->horizontalHeader()->setMinimumSectionSize(1);
     this->verticalHeader()->setMinimumSectionSize(1);
     this->setModel(model);
     pixelView = new PixelDelegate(this);
     blackwhiteView = new BlackWhiteDelegate(this);
     colorView= new ColorBarDelegate(this);
     this->setItemDelegate(colorView);

         minScale=0;
     maxScale=1;
       pixelSize=24;
       showItemNumber=false;
}

QPL_MatrixPlot::~QPL_MatrixPlot()
{

}

void QPL_MatrixPlot::setDelegate(matrixDelegate d){
    actualDelegate=d;
     switch(d)
        {
        case pixel:
         this->setItemDelegate(pixelView);
         pixelView->setMinScale(minScale);
         pixelView->setMaxScale(maxScale);
          pixelView->setPixelSize(pixelSize);
           pixelView->setShowItemNumber(showItemNumber);
         break;
        case blackwhite:
          this->setItemDelegate(blackwhiteView);
           blackwhiteView->setMinScale(minScale);
            blackwhiteView->setPixelSize(pixelSize);
             blackwhiteView->setShowItemNumber(showItemNumber);
          break;
         case colorbar:
           this->setItemDelegate(colorView);
           colorView->setMinScale(minScale);
             colorView->setMaxScale(maxScale);
              colorView->setPixelSize(pixelSize);
             colorView->setShowItemNumber(showItemNumber);
          break;
        default:
                break;
    }
}

  void QPL_MatrixPlot::setMinScale(double min)
 {
            minScale=min;
     switch(actualDelegate)
     {
        case pixel:
         pixelView->setMinScale(minScale);
         break;
        case blackwhite:
           blackwhiteView->setMinScale(minScale);
          break;
         case colorbar:
           colorView->setMinScale(minScale);
        default:
                break;
    }

 }

   void QPL_MatrixPlot::setMaxScale(double max)
 {
       maxScale=max;
     switch(actualDelegate)
     {
        case pixel:
         pixelView->setMaxScale(maxScale);
         break;
        case blackwhite:

          break;
         case colorbar:
           colorView->setMaxScale(maxScale);
        default:
                break;
    }
 }

      void QPL_MatrixPlot::setPixelSize(int psize){
          pixelSize=psize;
         switch(actualDelegate)
     {
        case pixel:
         pixelView->setPixelSize(pixelSize);
         break;
        case blackwhite:
           blackwhiteView->setPixelSize(pixelSize);
          break;
         case colorbar:
           colorView->setPixelSize(pixelSize);
        default:
                break;
    }


      }
void  QPL_MatrixPlot::setShowItemNumber(bool n){

              showItemNumber=n;
         switch(actualDelegate)
     {
        case pixel:
         pixelView->setShowItemNumber(showItemNumber);
         break;
        case blackwhite:
           blackwhiteView->setShowItemNumber(showItemNumber);
          break;
         case colorbar:
         colorView->setShowItemNumber(showItemNumber);
        default:
                break;
    }


}


void QPL_MatrixPlot::setValue(const QVector< QVector<float> > &v){
//for (int i=0;i<v.size();i++){
//        for (int j=0;j<v.at(i).size();j++){
////             printf(" %f ",v.at(i).at(j));
//             (matrixPlot->item(i,j))->setText(tr("%1").arg(v.at(i).at(j)));
//         }
////         printf("\n");
//}
//  printf("\n");

    model->setData(v);
    this->resizeColumnsToContents();
    this->resizeRowsToContents();
}


   QDataStream& operator<<(QDataStream &out, const QPL_MatrixPlot &d){
        qint32 a;

          out << d.minScale;
          out << d.maxScale;
          a=d.pixelSize; out << a;
          out << d.showItemNumber;

        return out;
}


QDataStream& operator>>(QDataStream &in, QPL_MatrixPlot(&d)){

        bool b;  qint32 a; double dd;


        in >> dd; d.setMinScale(dd);
        in >> dd; d.setMaxScale(dd);
        in >> a; d.setPixelSize(a);
        in >> b; d.setShowItemNumber(b);



        return in;
}
