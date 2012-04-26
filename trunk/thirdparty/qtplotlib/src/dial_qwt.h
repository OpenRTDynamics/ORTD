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

#ifndef DIAL_QWT_H
#define DIAL_QWT_H

#include "qtplot_global.h"
#include <qwt_dial.h>
#include <qwt_dial_needle.h>

class  QPL_DialQwt : public QwtDial
{
   Q_OBJECT

public:
   QPL_DialQwt(QWidget *parent = 0);
   virtual ~QPL_DialQwt();
      void setMin(double);
   double getMin(){return Min;}
   void setMax(double);
    double getMax(){return Max;}
       void setNeedleColor(const QColor&);
   QColor getNeedleColor(){return needle->palette().button().color();}
    private:
      double Max,Min;
        QwtDialSimpleNeedle *needle;
            friend QDataStream& operator<<(QDataStream &out, const QPL_DialQwt &d);
  friend QDataStream& operator>>(QDataStream &in, QPL_DialQwt(&d));
};
        QDataStream& operator<<(QDataStream &out, const QPL_DialQwt &d);
        QDataStream& operator>>(QDataStream &in, QPL_DialQwt(&d));

#endif // DIAL_QWT_H
