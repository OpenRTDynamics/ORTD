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

#include "thermo_qwt.h"

QPL_ThermoQwt::QPL_ThermoQwt(QWidget *parent)
        :QwtThermo(parent)
{
    // this->setObjectName(QString::fromUtf8("Thermo"));
    //this->setGeometry(QRect(50, 20, 52, 261));


        Min=-1.;
        Max=1.;

    this->setRange(Min,Max);
  //  Thermo->setScale(Min,Max);
    setPipeWidth(14);
    this->setAutoScale();
     this->setScaleMaxMajor(5);
    this->setScaleMaxMinor(10);
     pipeDistance=this->minimumSizeHint().width()-this->pipeWidth()-this->borderWidth()*2;

    thermoColor2=Qt::black;
    thermoColor1=Qt::red;
     gradient=QLinearGradient(47, 0, 52, 0);
     gradient.setColorAt(0, thermoColor1);
     gradient.setColorAt(1, thermoColor2);
     gradient.setSpread(QGradient::ReflectSpread);

this->setFillBrush(QBrush(gradient));

        alarmThermoColor1=Qt::blue;
        alarmThermoColor2=Qt::black;
        alarmGradient=QLinearGradient(QPointF(pipeDistance+pipeWidth()/2,0), QPointF(pipeDistance+pipeWidth(),0));
                alarmGradient.setColorAt(0, alarmThermoColor1);
                alarmGradient.setColorAt(1, alarmThermoColor2);
                alarmGradient.setSpread(QGradient::ReflectSpread);
                this->setAlarmBrush(QBrush(alarmGradient));

        this->setAlarmLevel(1);
        setPipeWith(pipeWidth());
        gradientEnabled=true;
}

QPL_ThermoQwt::~QPL_ThermoQwt(){





}
void QPL_ThermoQwt::setMax(double max)
{
        Max=max;
        this->setMaxValue(Max);
        pipeDistance=this->minimumSizeHint().width()-this->pipeWidth()-this->borderWidth()*2;
        setPipeWith(pipeWidth());
        //Dial->setRange(Min,Max);
}


void QPL_ThermoQwt::setMin(double min)
{
        Min=min;
        this->setMinValue(Min);
        pipeDistance=this->minimumSizeHint().width()-this->pipeWidth()-this->borderWidth()*2;
        setPipeWith(pipeWidth());
       // Dial->setRange(Min,Max);
}

void QPL_ThermoQwt::setThermoColor1(const QColor& c1)
{
        thermoColor1=c1;
        //Thermo->setFillColor(thermoColor1);

          gradient=QLinearGradient(QPointF(pipeDistance+pipeWidth()/2,0), QPointF(pipeDistance+pipeWidth(),0));
        if(gradientEnabled)
                gradient.setColorAt(1, thermoColor2);
        else
                gradient.setColorAt(1, thermoColor1);
     gradient.setColorAt(0, thermoColor1);
     gradient.setSpread(QGradient::ReflectSpread);
        this->setFillBrush(QBrush(gradient));
}


void QPL_ThermoQwt::setThermoColor2(const QColor& c2)
{
        thermoColor2=c2;
        //Thermo->setFillColor(c2);

          gradient=QLinearGradient(QPointF(pipeDistance+pipeWidth()/2,0), QPointF(pipeDistance+pipeWidth(),0));
        if(gradientEnabled)
                gradient.setColorAt(1, thermoColor2);
        else
                gradient.setColorAt(1, thermoColor1);
     gradient.setColorAt(0, thermoColor1);
     gradient.setSpread(QGradient::ReflectSpread);
        this->setFillBrush(QBrush(gradient));
}

void QPL_ThermoQwt::setPipeWith(int pipewidth)
{
        QwtThermo::setPipeWidth(pipewidth);
         this->resize(pipeDistance+pipeWidth()+2*this->borderWidth()+50,this->size().height());
         gradient.setStart(QPointF(pipeDistance+pipeWidth()/2,0));
         gradient.setFinalStop(QPointF(pipeDistance+pipeWidth(),0));
        this->setFillBrush(QBrush(gradient));
        alarmGradient.setStart(QPointF(pipeDistance+pipeWidth()/2,0));
         alarmGradient.setFinalStop(QPointF(pipeDistance+pipeWidth(),0));
        this->setAlarmBrush(QBrush(alarmGradient));
}



void QPL_ThermoQwt::setAlarmThermoColor1(const QColor& c1)
{
        alarmThermoColor1=c1;
        //Thermo->setFillColor(thermoColor1);
          alarmGradient=QLinearGradient(QPointF(pipeDistance+pipeWidth()/2,0), QPointF(pipeDistance+pipeWidth(),0));
        if(gradientEnabled)
                     alarmGradient.setColorAt(1, alarmThermoColor2);
        else
                     alarmGradient.setColorAt(1, alarmThermoColor1);
     alarmGradient.setColorAt(0, alarmThermoColor1);
     alarmGradient.setSpread(QGradient::ReflectSpread);
        this->setAlarmBrush(QBrush(alarmGradient));
}


void QPL_ThermoQwt::setAlarmThermoColor2(const QColor& c2)
{
        alarmThermoColor2=c2;
        //Thermo->setFillColor(c2);
          alarmGradient=QLinearGradient(QPointF(pipeDistance+pipeWidth()/2,0), QPointF(pipeDistance+pipeWidth(),0));
        if(gradientEnabled)
                     alarmGradient.setColorAt(1, alarmThermoColor2);
        else
                     alarmGradient.setColorAt(1, alarmThermoColor1);
     alarmGradient.setColorAt(0, alarmThermoColor1);
     alarmGradient.setSpread(QGradient::ReflectSpread);
        this->setAlarmBrush(QBrush(alarmGradient));
}

void QPL_ThermoQwt::setThermoAlarm(int state)
{
        if (state==Qt::Checked){
                this->setAlarmEnabled(true);
                this->setAlarmLevel(alarmLevel());
                alarmGradient=QLinearGradient(QPointF(pipeDistance+pipeWidth()/2,0), QPointF(pipeDistance+pipeWidth(),0));
                if(gradientEnabled)
                alarmGradient.setColorAt(1, alarmThermoColor2);
                else
                alarmGradient.setColorAt(1, alarmThermoColor1);
                alarmGradient.setColorAt(0, alarmThermoColor1);
                alarmGradient.setSpread(QGradient::ReflectSpread);
                this->setAlarmBrush(QBrush(alarmGradient));
        } else
                this->setAlarmEnabled(false);

}

void QPL_ThermoQwt::setThermoAlarm(bool b)
{
        if (b){
                this->setAlarmEnabled(true);
                this->setAlarmLevel(alarmLevel());
                alarmGradient=QLinearGradient(QPointF(pipeDistance+pipeWidth()/2,0), QPointF(pipeDistance+pipeWidth(),0));
                if(gradientEnabled)
                alarmGradient.setColorAt(1, alarmThermoColor2);
                else
                alarmGradient.setColorAt(1, alarmThermoColor1);
                alarmGradient.setColorAt(0, alarmThermoColor1);
                alarmGradient.setSpread(QGradient::ReflectSpread);
                this->setAlarmBrush(QBrush(alarmGradient));
        } else
                this->setAlarmEnabled(false);

}



void QPL_ThermoQwt::setGradientEnabled(bool b)
{
        gradientEnabled=b;
        if(!gradientEnabled){
                alarmGradient=QLinearGradient(QPointF(pipeDistance+pipeWidth()/2,0), QPointF(pipeDistance+pipeWidth(),0));
                alarmGradient.setColorAt(0, alarmThermoColor1);
                alarmGradient.setColorAt(1, alarmThermoColor1);
                alarmGradient.setSpread(QGradient::ReflectSpread);
                this->setAlarmBrush(QBrush(alarmGradient));

                gradient=QLinearGradient(QPointF(pipeDistance+pipeWidth()/2,0), QPointF(pipeDistance+pipeWidth(),0));
                gradient.setColorAt(0, thermoColor1);
                gradient.setColorAt(1, thermoColor1);
                gradient.setSpread(QGradient::ReflectSpread);
                this->setFillBrush(QBrush(gradient));
        } else {

                alarmGradient=QLinearGradient(QPointF(pipeDistance+pipeWidth()/2,0), QPointF(pipeDistance+pipeWidth(),0));
                alarmGradient.setColorAt(0, alarmThermoColor1);
                alarmGradient.setColorAt(1, alarmThermoColor2);
                alarmGradient.setSpread(QGradient::ReflectSpread);
                this->setAlarmBrush(QBrush(alarmGradient));

                gradient=QLinearGradient(QPointF(pipeDistance+pipeWidth()/2,0), QPointF(pipeDistance+pipeWidth(),0));
                gradient.setColorAt(0, thermoColor1);
                gradient.setColorAt(1, thermoColor2);
                gradient.setSpread(QGradient::ReflectSpread);
                this->setFillBrush(QBrush(gradient));

        }
}



void  QPL_ThermoQwt::setThermoDirection(Qt::Orientation o)
{
        if(o==Qt::Vertical)
                this->setOrientation(o, QwtThermo::LeftScale);
        else
                this->setOrientation(o, QwtThermo::BottomScale);
}


QDataStream& operator<<(QDataStream &out, const QPL_ThermoQwt &d){


        out << d.gradientEnabled;
        out << d.thermoColor1;
        out << d.thermoColor2;
        out << d.alarmThermoColor1;
        out << d.alarmThermoColor2;
        out << (qint32)d.pipeWidth();
        out << d.Min << d.Max;
        out << d.alarmLevel() << d.alarmEnabled();
    }

QDataStream& operator>>(QDataStream &in, QPL_ThermoQwt(&d)){
        QSize s;QPoint p;bool b; QColor c; qint32 a;QFont f; double dd;
        QChar ch;

        in >> b; d.setGradientEnabled(b);
        in >> c; d.setThermoColor1(c);
        in >> c; d.setThermoColor2(c);
        in >> c; d.setAlarmThermoColor1(c);
        in >> c; d.setAlarmThermoColor2(c);
        in >> a; d.setPipeWith(a);
        in >> dd; d.setMin(dd);
        in >> dd;  d.setMax(dd);
        in >> dd;d.setAlarmLevel(dd);
        in >> b; d.setThermoAlarm(b);
    }
