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

#ifndef LOG_MATRIXVIEW_H
#define LOG_MATRIXVIEW_H

#include <QtGui>

#include "qtplot_global.h"

 class PixelDelegate : public QAbstractItemDelegate
 {
     Q_OBJECT

 public:
     PixelDelegate(QObject *parent = 0);

     void paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const;

     QSize sizeHint(const QStyleOptionViewItem &option,
                    const QModelIndex &index ) const;
     void setMinScale(double min);
     void setMaxScale(double max);
     void setPixelSize(int size);
    void  setShowItemNumber(bool n){showItemNumber=n;}
     void setShowValue(bool n){showValue=n;}

 private:
     int pixelSize;
     double minScale;
     double maxScale;
     bool showItemNumber;
          bool showValue;
 };

  class BlackWhiteDelegate : public QAbstractItemDelegate
 {
     Q_OBJECT

 public:
     BlackWhiteDelegate(QObject *parent = 0);

     void paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const;

     QSize sizeHint(const QStyleOptionViewItem &option,
                    const QModelIndex &index ) const;
     void setMinScale(double min);
 void setPixelSize(int size);
    void  setShowItemNumber(bool n){showItemNumber=n;}
     void setShowValue(bool n){showValue=n;}
 private:
     int pixelSize;
     double minScale;
     bool showItemNumber;
          bool showValue;
 };

    class ColorBarDelegate : public QAbstractItemDelegate
 {
     Q_OBJECT

 public:
     ColorBarDelegate(QObject *parent = 0);

     void paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const;

     QSize sizeHint(const QStyleOptionViewItem &option,
                    const QModelIndex &index ) const;
     void setMinScale(double min);
     void setMaxScale(double max);
    void setPixelSize(int size);
     void  setShowItemNumber(bool n){showItemNumber=n;}
     void setShowValue(bool n){showValue=n;}
 private:
     int pixelSize;
     double minScale;
     double maxScale;
     bool showItemNumber;
     bool showValue;
 };

 class MatrixModel : public QAbstractTableModel
 {
     Q_OBJECT

 public:
     MatrixModel(QObject *parent = 0);

     void setData(const QVector< QVector<float> > &v);

     int rowCount(const QModelIndex &parent = QModelIndex()) const;
     int columnCount(const QModelIndex &parent = QModelIndex()) const;

     QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
     QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

 private:
     QVector< QVector<float> > matrixData;

 };

#endif // LOG_MATRIXVIEW_H
