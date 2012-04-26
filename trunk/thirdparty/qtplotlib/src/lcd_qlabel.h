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

#ifndef LCD_QLABEL_H
#define LCD_QLABEL_H

#include "qtplot_global.h"
#include <QtGui>

class  QPL_LcdQLabel : public QLabel
{
   Q_OBJECT

public:
   QPL_LcdQLabel(QWidget *parent = 0);
   virtual ~QPL_LcdQLabel();
     void setLcdFont(const QFont& font);
   QFont getLcdFont(){return this->font();}
  void setLcdFormat(char c) ;
  char getLcdFormat() {return format;}
  void setLcdPrecision(int p);
   int getLcdPrecision(){return precision;}
   void setValue(double);
private:
     int precision;
  char format;
              friend QDataStream& operator<<(QDataStream &out, const QPL_LcdQLabel &d);
  friend QDataStream& operator>>(QDataStream &in, QPL_LcdQLabel(&d));
};
        QDataStream& operator<<(QDataStream &out, const QPL_LcdQLabel &d);
        QDataStream& operator>>(QDataStream &in, QPL_LcdQLabel(&d));

#endif // LCD_QLABEL_H
