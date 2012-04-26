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

#include "led.h"


QPL_Led::QPL_Led(QWidget *parent)
    : QWidget(parent)
{
   m_value=false;
   m_color=Qt::red;
   m_str=tr("");
   setMinimumSize(QSize(50,50));
   //setFocusPolicy(Qt::StrongFocus);
   scaledSize = 150;

}

void QPL_Led::paintEvent(QPaintEvent *)
{

        //QSvgRenderer *renderer = new QSvgRenderer();
        qreal realSize = qMin(width(), height());
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        //painter.setWindow( -50,-50,100,100);
    painter.translate(width()/2, height()/2);
    painter.scale(realSize/scaledSize, realSize/scaledSize);

        painter.setPen(Qt::white);
        painter.drawArc(-25,-25,50,50,0,5670);
        painter.drawArc(-32,-33,66,66,0,5670);
        painter.setPen(Qt::darkGray);
        painter.drawArc(-34,-33,66,66,3400,3000);
        if (!m_str.isEmpty())
            painter.drawText(50,10,m_str);

    if(m_value)
    {
      QRadialGradient radialGrad(QPointF(-8, -8), 20);
      radialGrad.setColorAt(0, Qt::white);

      radialGrad.setColorAt(1, m_color);
          QBrush brush(radialGrad);
      painter.setBrush(brush);
      painter.setPen(Qt::black);
          painter.drawEllipse(-25,-25,50,50);
    }
    else
    {
      QRadialGradient radialGrad(QPointF(-8, -8), 20);
      radialGrad.setColorAt(0, Qt::white);
      radialGrad.setColorAt(1, Qt::lightGray);
          QBrush brush(radialGrad);
      painter.setBrush(brush);
          painter.drawEllipse(-25,-25,50,50);
    }
 //renderer->render(&painter);

}

void QPL_Led::setColor(QColor newColor)
{
   m_color=newColor;
   update();
}

void QPL_Led::setText(QString newStr)
{
   m_str=newStr;
   update();
}


void QPL_Led::setValue(bool value)
{
   m_value=value;
   update();
}


void QPL_Led::toggleValue()
{
        m_value=!m_value;
        update();
}
