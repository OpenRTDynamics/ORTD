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

#include "lcd_qlabel.h"

QPL_LcdQLabel::QPL_LcdQLabel(QWidget *parent)
        :QLabel(parent)
{
precision=4;
        format='f';

         QFont font("Helvetica", 15, QFont::DemiBold);
        this->setFont(font);
        this->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
}

QPL_LcdQLabel::~QPL_LcdQLabel(){

}

   void QPL_LcdQLabel::setLcdFont(const QFont& font){
        this->setFont(font);

}
void QPL_LcdQLabel::setLcdPrecision(int p) {
  precision=p;

}

void QPL_LcdQLabel::setLcdFormat(char c) {
  switch(c){
  case 'e':	this->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
     format=c;
    break;
  case 'f': 	this->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
     format=c;
    break;
  case 'g':	this->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
     format=c;
    break;
  }
}

void QPL_LcdQLabel::setValue(double v)
{
                QLocale loc;
                QString str=loc.toString(v,format,precision);
                if (v >= 0)
                        str.insert(0,QString(" "));
                this->setText(str);
}


   QDataStream& operator<<(QDataStream &out, const QPL_LcdQLabel &d){


        out << (qint32)d.precision << (QChar)d.format;
        return out;
}


QDataStream& operator>>(QDataStream &in, QPL_LcdQLabel(&d)){
        QChar ch;qint32 a;


          in >> a; d.setLcdPrecision(a);
          in >> ch; d.setLcdFormat(ch.toAscii());


        return in;
}
