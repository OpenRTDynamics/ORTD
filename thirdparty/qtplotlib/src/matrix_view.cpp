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

#include "matrix_view.h"


PixelDelegate::PixelDelegate(QObject *parent)
     : QAbstractItemDelegate(parent)
 {
     pixelSize = 24;
     minScale=0;
     maxScale=1;
     showItemNumber=false;
     showValue=false;
 }

 void PixelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
 {
     if (option.state & QStyle::State_Selected)
         painter->fillRect(option.rect, option.palette.highlight());


     int size = qMin(option.rect.width(), option.rect.height());
     double normData = (index.model()->data(index, Qt::DisplayRole).toDouble()-minScale)/(maxScale-minScale);
     if (normData<0)
               normData=0;
     if (normData>1)
            normData=1;
     normData=1-normData;
     double radius = (size/2.0) - (normData * size/2.0);

     painter->save();
     painter->setRenderHint(QPainter::Antialiasing, true);

     painter->setPen(Qt::gray);
     painter->setBrush(QBrush(Qt::white));
         radius = (size/2.0);
      painter->drawRect(QRectF(option.rect.x() + option.rect.width()/2 - radius,
                                 option.rect.y() + option.rect.height()/2 - radius,
                                 2*radius, 2*radius));
      radius = (size/2.0) - (normData * size/2.0);
       painter->setPen(Qt::NoPen);
     if (option.state & QStyle::State_Selected)
         painter->setBrush(option.palette.highlightedText());
     else
         painter->setBrush(QBrush(Qt::black));

    if (radius > 0.0){
     painter->drawEllipse(QRectF(option.rect.x() + option.rect.width()/2 - radius,
                                 option.rect.y() + option.rect.height()/2 - radius,
                                 2*radius, 2*radius));
 }

          int number=0;
          radius = (size/2.0);
     if (showItemNumber) {
         painter->setPen(Qt::gray);
         painter->setFont(QFont("Arial", 10));
         number=index.row()*index.model()->columnCount()+index.column();
         painter->drawText(QRectF(option.rect.x() + option.rect.width()/2 - radius,
                                     option.rect.y() + option.rect.height()/2 - radius,
                                     2*radius, 2*radius),tr("%1").arg(number));
        }else if (showValue) {
         painter->setPen(Qt::black);
         painter->setFont(QFont("Arial", 8));

         painter->drawText(QRectF(option.rect.x() + option.rect.width()/2 + 2*radius,
                                     option.rect.y() + option.rect.height()/2 -radius/2,
                                     8*radius, 2*radius),tr("%1").arg(index.model()->data(index, Qt::DisplayRole).toDouble()));

        }
     painter->restore();
 }

 QSize PixelDelegate::sizeHint(const QStyleOptionViewItem & /* option */,
                               const QModelIndex & /* index */) const
 {
     return QSize(pixelSize, pixelSize);
 }

 void PixelDelegate::setPixelSize(int size)
 {
     pixelSize = size;
 }

  void PixelDelegate::setMinScale(double min)
 {
     minScale = min;
 }

   void PixelDelegate::setMaxScale(double max)
 {
     maxScale = max;
 }



BlackWhiteDelegate::BlackWhiteDelegate(QObject *parent)
     : QAbstractItemDelegate(parent)
 {
     pixelSize = 24;
     minScale=0;
     showItemNumber=false;
          showValue=false;
 }

 void BlackWhiteDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
 {
     if (option.state & QStyle::State_Selected)
         painter->fillRect(option.rect, option.palette.highlight());


     int size = qMin(option.rect.width(), option.rect.height());

     double radius = (size/2.0) ;


     painter->save();
     painter->setRenderHint(QPainter::Antialiasing, true);
     painter->setPen(Qt::NoPen);
     if (option.state & QStyle::State_Selected)
         painter->setBrush(option.palette.highlightedText());
     else {
         if (index.model()->data(index, Qt::DisplayRole).toDouble()<=minScale)
            painter->setBrush(QBrush(Qt::black));
         else
          painter->setBrush(QBrush(Qt::white));
     }
     int number=0;
              painter->setPen(Qt::gray);
          painter->drawRect(QRectF(option.rect.x() + option.rect.width()/2 - radius,
                                 option.rect.y() + option.rect.height()/2 - radius,
                                 2*radius, 2*radius));
     if (showItemNumber) {
         painter->setPen(Qt::gray);
         painter->setFont(QFont("Arial", 10));
         number=index.row()*index.model()->columnCount()+index.column();
         painter->drawText(QRectF(option.rect.x() + option.rect.width()/2 - radius,
                                     option.rect.y() + option.rect.height()/2 - radius,
                                     2*radius, 2*radius),tr("%1").arg(number));
        }else if (showValue) {
         painter->setPen(Qt::black);
         painter->setFont(QFont("Arial", 8));

         painter->drawText(QRectF(option.rect.x() + option.rect.width()/2 + 2*radius,
                                     option.rect.y() + option.rect.height()/2 -radius/2,
                                     8*radius, 2*radius),tr("%1").arg(index.model()->data(index, Qt::DisplayRole).toDouble()));

        }

     painter->restore();
 }

 QSize BlackWhiteDelegate::sizeHint(const QStyleOptionViewItem & /* option */,
                               const QModelIndex & /* index */) const
 {
     return QSize(pixelSize, pixelSize);
 }


  void BlackWhiteDelegate::setMinScale(double min)
 {
     minScale = min;
 }

 void BlackWhiteDelegate::setPixelSize(int size)
 {
     pixelSize = size;
 }

  ColorBarDelegate::ColorBarDelegate(QObject *parent)
     : QAbstractItemDelegate(parent)
 {
     pixelSize = 24;
     minScale=0;
     maxScale=1;
     showItemNumber=false;
     showValue=false;
 }

 void ColorBarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
 {
     if (option.state & QStyle::State_Selected)
         painter->fillRect(option.rect, option.palette.highlight());


     int size = qMin(option.rect.width(), option.rect.height());

     double radius = (size/2.0) ;


     painter->save();
     painter->setRenderHint(QPainter::Antialiasing, true);
     //painter->setPen(Qt::NoPen);
     QPen pen(Qt::gray);
    // pen.setStyle(Qt::DashLine);
     painter->setPen(pen);
       double normData = (index.model()->data(index, Qt::DisplayRole).toDouble()-minScale)/(maxScale-minScale);
       if (normData<0)
               normData=0;
       if (normData>1)
            normData=1;
       normData=1-normData;
        QColor color;
        color.setHsv(normData*240,240,240);
     if (option.state & QStyle::State_Selected)
         painter->setBrush(option.palette.highlightedText());
     else
           painter->setBrush(QBrush(color));


     painter->drawRect(QRectF(option.rect.x() + option.rect.width()/2 - radius,
                                 option.rect.y() + option.rect.height()/2 - radius,
                                 2*radius, 2*radius));
     int number=0;
     if (showItemNumber) {
         painter->setPen(Qt::white);
         painter->setFont(QFont("Arial", 10));
         number=index.row()*index.model()->columnCount()+index.column();
         painter->drawText(QRectF(option.rect.x() + option.rect.width()/2 - radius,
                                     option.rect.y() + option.rect.height()/2 - radius,
                                     2*radius, 2*radius),tr("%1").arg(number));
        } else if (showValue) {
         painter->setPen(Qt::black);
         painter->setFont(QFont("Arial", 8));

         painter->drawText(QRectF(option.rect.x() + option.rect.width()/2 + 2*radius,
                                     option.rect.y() + option.rect.height()/2 -radius/2,
                                     8*radius, 2*radius),tr("%1").arg(index.model()->data(index, Qt::DisplayRole).toDouble()));

        }
     painter->restore();
 }

 QSize ColorBarDelegate::sizeHint(const QStyleOptionViewItem & /* option */,
                               const QModelIndex & /* index */) const
 {
     return QSize(pixelSize, pixelSize);
 }


  void ColorBarDelegate::setMinScale(double min)
 {
     minScale = min;
 }

    void ColorBarDelegate::setMaxScale(double max)
 {
     maxScale = max;
 }

 void ColorBarDelegate::setPixelSize(int size)
 {
     pixelSize = size;
 }

 MatrixModel::MatrixModel(QObject *parent)
     : QAbstractTableModel(parent)
 {
 }


 void MatrixModel::setData(const QVector< QVector<float> > &v)
 {
     matrixData = v;
     reset();
 }

 int MatrixModel::rowCount(const QModelIndex & /* parent */) const
 {
     return matrixData.size();
 }

 int MatrixModel::columnCount(const QModelIndex & /* parent */) const
 {
     if (matrixData.size()>0)
        return matrixData.at(0).size();
     else
        return 0;
 }

 QVariant MatrixModel::data(const QModelIndex &index, int role) const
 {
     if (!index.isValid() || role != Qt::DisplayRole)
         return QVariant();
     return matrixData.at(index.row()).at(index.column());
 }

 QVariant MatrixModel::headerData(int /* section */,
                                 Qt::Orientation /* orientation */,
                                 int role) const
 {
     if (role == Qt::SizeHintRole)
         return QSize(1, 1);
     return QVariant();
 }







