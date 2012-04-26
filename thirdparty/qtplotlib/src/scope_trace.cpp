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

/*
 file:		scopes_trace.cpp
 describtion:
   file for the classes QRL_ScopeTrace
*/

#include "scope_trace.h"
#include <stdlib.h>

QPL_ScopeTrace::QPL_ScopeTrace(QwtPlot *parent, unsigned int maxdatapoints, int j){
		qwtPlot=parent;
		MaxDataPoints=maxdatapoints;
                //index = j;
                index=0;
		d_x = new double[MaxDataPoints+1];
		d_y = new double[MaxDataPoints+1];

		for (unsigned int i = 0; i< MaxDataPoints; i++)
	      {
		  d_x[i] =0.;     // time axis
		  d_y[i] = 0.;
	      }
		cData = new QwtPlotCurve(QObject::tr("Trace %1").arg(j));
	 	cData->attach(qwtPlot);
                //cData->setPaintAttribute(QwtPlotCurve::PaintFiltered,true);
                cData->setPaintAttribute(QwtPlotCurve::ClipPolygons,true);
                oldStyle=QwtPlotCurve::Lines;
                cData->setStyle(QwtPlotCurve::Lines);
                setSymbolStyle(QwtSymbol::NoSymbol);
                oldSymbol=QwtSymbol::NoSymbol;
		xmax=1;
		lineWidth=3;
		dy=1.;
		offset=0.;
		average=0.;
		min=0.;
		max=0.;
		PP=0.;
		RMS=0.;
                labelOffset=2.5;
		visible=true;
		labelCounter=0;
		switch(j){
			case 0:brush=QBrush(Qt::red);
				break;
			case 1:brush=QBrush(Qt::green);
				break;
			case 2:brush=QBrush(Qt::yellow);
				break;
			case 3:brush=QBrush(Qt::blue);
				break;
			case 4:brush=QBrush(Qt::black);
				break;
			case 5:brush=QBrush(Qt::magenta);
				break;
			case 6:brush=QBrush(Qt::darkRed);
				break;
			case 7:brush=QBrush(Qt::darkGreen);
				break;
			case 8:brush=QBrush(Qt::darkMagenta);
				break;
			case 9:brush=QBrush(Qt::darkYellow);
				break;
			case 10:brush=QBrush(Qt::cyan);
				break;
			case 11:brush=QBrush(Qt::darkCyan);
				break;
			case 12:brush=QBrush(Qt::gray);
				break;
			case 13:brush=QBrush(Qt::darkGray);
				break;
			case 14:brush=QBrush(Qt::lightGray);
				break;
		}

		QPen pen;
       		pen.setWidth(lineWidth);
		pen.setBrush(brush);
		cData->setPen(pen);
		//cData->setRawData(d_x, d_y, NDataSoll);


		time=0;

		zeroAxis.setLabel(QObject::tr("%1").arg(j));    
		zeroAxis.setLabelAlignment(Qt::AlignLeft|Qt::AlignTop);
    		zeroAxis.setLineStyle(QwtPlotMarker::HLine);
    		//zeroAxis.setYValue(offset/dy);
		zeroAxis.setYValue(offset);
    		zeroAxis.setLinePen(QPen(brush,2.,Qt::DashDotLine));
    		zeroAxis.attach(qwtPlot);
		zeroAxis.hide();

		traceName=QObject::tr("Trace %1").arg(j);
		QwtText ttext(traceName);
   		 ttext.setColor(QColor(brush.color()));
    		traceLabel.setLabel(ttext);
		traceLabel.setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
    		traceLabel.setLineStyle(QwtPlotMarker::NoLine);
    		//zeroAxis.setYValue(offset/dy);
		traceLabel.setXValue(0.+j*xmax/5.);
                traceLabel.setYValue(3.);
		
    		traceLabel.setLinePen(QPen(brush,2.,Qt::DashDotLine));
    		traceLabel.attach(qwtPlot);
		traceLabel.hide();

		QwtText unitText(QObject::tr("%1").arg(0.));
   		 unitText.setColor(QColor(gridColor));
    		unitLabel.setLabel(unitText);
		unitLabel.setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
    		unitLabel.setLineStyle(QwtPlotMarker::NoLine);
    		//zeroAxis.setYValue(offset/dy);
		unitLabel.setXValue(0.+j*xmax/5.);
                unitLabel.setYValue(labelOffset);
		
    		unitLabel.setLinePen(QPen(brush,2.,Qt::DashDotLine));
    		unitLabel.attach(qwtPlot);
		unitLabel.hide();


		QwtText atext(QObject::tr("%1").arg(0.));
   		 atext.setColor(QColor(gridColor));
    		averageLabel.setLabel(atext);
		averageLabel.setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
    		averageLabel.setLineStyle(QwtPlotMarker::NoLine);
    		//zeroAxis.setYValue(offset/dy);
		averageLabel.setXValue(0.+j*xmax/5.);
                averageLabel.setYValue(labelOffset);
		
    		averageLabel.setLinePen(QPen(brush,2.,Qt::DashDotLine));
    		averageLabel.attach(qwtPlot);
		averageLabel.hide();
		
		QwtText minText(QObject::tr("%1").arg(0.));
   		 minText.setColor(QColor(gridColor));
    		minLabel.setLabel(minText);
		minLabel.setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
    		minLabel.setLineStyle(QwtPlotMarker::NoLine);
    		//zeroAxis.setYValue(offset/dy);
		minLabel.setXValue(0.+j*xmax/5.);
                minLabel.setYValue(labelOffset);
		
    		minLabel.setLinePen(QPen(brush,2.,Qt::DashDotLine));
    		minLabel.attach(qwtPlot);
		minLabel.hide();

		QwtText maxText(QObject::tr("%1").arg(0.));
   		 maxText.setColor(QColor(gridColor));
    		maxLabel.setLabel(maxText);
		maxLabel.setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
    		maxLabel.setLineStyle(QwtPlotMarker::NoLine);
    		//zeroAxis.setYValue(offset/dy);
		maxLabel.setXValue(0.+j*xmax/5.);
                maxLabel.setYValue(labelOffset);
		
    		maxLabel.setLinePen(QPen(brush,2.,Qt::DashDotLine));
    		maxLabel.attach(qwtPlot);
		maxLabel.hide();

		QwtText ppText(QObject::tr("%1").arg(0.));
   		 ppText.setColor(QColor(gridColor));
    		ppLabel.setLabel(ppText);
		ppLabel.setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
    		ppLabel.setLineStyle(QwtPlotMarker::NoLine);
    		//zeroAxis.setYValue(offset/dy);
		ppLabel.setXValue(0.+j*xmax/5.);
                ppLabel.setYValue(labelOffset);
		
    		ppLabel.setLinePen(QPen(brush,2.,Qt::DashDotLine));
    		ppLabel.attach(qwtPlot);
		ppLabel.hide();

		QwtText rmsText(QObject::tr("%1").arg(0.));
   		 rmsText.setColor(QColor(gridColor));
    		rmsLabel.setLabel(rmsText);
		rmsLabel.setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
    		rmsLabel.setLineStyle(QwtPlotMarker::NoLine);
    		//zeroAxis.setYValue(offset/dy);
		rmsLabel.setXValue(0.+j*xmax/5.);
                rmsLabel.setYValue(labelOffset);
		
    		rmsLabel.setLinePen(QPen(brush,2.,Qt::DashDotLine));
    		rmsLabel.attach(qwtPlot);
		rmsLabel.hide();




}

QPL_ScopeTrace::~QPL_ScopeTrace(){
//Plotting_Scope_Data_Thread->stopThread();
//Plotting_Scope_Data_Thread->wait();
//delete Plotting_Scope_Data_Thread;
//delete mY;
  delete[] d_x;
  delete[] d_y;
  delete cData;

}


void QPL_ScopeTrace::refresh()
{


if ( isLabelVisible(lt_average)){
	average=0.;
        for (unsigned int k=0;k<NDataSoll;k++)
		average+=(d_y[k]-offset)*dy/((double)NDataSoll);

	QString astr;
	astr.setNum(average,'f',3);
	//QwtText atext(QObject::tr("Avg: %1").arg(average));
	QwtText atext(QObject::tr("Avg: ")+astr);
	atext.setColor(QColor(gridColor));
	averageLabel.setLabel(atext);
}
if ( isLabelVisible(lt_unit)){
	QString astr;
	astr.setNum(dy,'f',3);
	//QwtText atext(QObject::tr("Avg: %1").arg(average));
	QwtText atext(QObject::tr("U/d: ")+astr);
	atext.setColor(QColor(gridColor));
	unitLabel.setLabel(atext);
}
if ( isLabelVisible(lt_min)){
	min=(d_y[0]-offset)*dy;
        for (unsigned int k=0;k<NDataSoll;k++)
		if (min>(d_y[k]-offset)*dy)
			min=(d_y[k]-offset)*dy;

	QString astr;
	astr.setNum(min,'f',3);
	QwtText atext(QObject::tr("Min: ")+astr);
   	atext.setColor(QColor(gridColor));
    	minLabel.setLabel(atext);
}
if ( isLabelVisible(lt_max)){
	max=(d_y[0]-offset)*dy;
        for (unsigned int k=0;k<NDataSoll;k++)
		if (max<(d_y[k]-offset)*dy)
			max=(d_y[k]-offset)*dy;

	QString astr;
	astr.setNum(max,'f',3);
	QwtText atext(QObject::tr("Max: ")+astr);
   	atext.setColor(QColor(gridColor));
    	maxLabel.setLabel(atext);
}
if ( isLabelVisible(lt_pp)){
	min=(d_y[0]-offset)*dy;
	max=(d_y[0]-offset)*dy;
	for (int k=0;k<NDataSoll;k++){
		if (max<(d_y[k]-offset)*dy)
			max=(d_y[k]-offset)*dy;
		if (min>(d_y[k]-offset)*dy)
			min=(d_y[k]-offset)*dy;
	}
	PP=max-min;
	QString astr;
	astr.setNum(PP,'f',3);
	QwtText atext(QObject::tr("PP: ")+astr);
   	atext.setColor(QColor(gridColor));
    	ppLabel.setLabel(atext);
}
if ( isLabelVisible(lt_rms)){
	RMS=0.;
        for (unsigned int k=0;k<NDataSoll;k++)
		RMS+=(d_y[k]-offset)*dy*(d_y[k]-offset)*dy;
	RMS=sqrt(RMS/((double)NDataSoll));
	QString astr;
	astr.setNum(RMS,'f',3);
	QwtText atext(QObject::tr("RMS: ")+astr);
	atext.setColor(QColor(gridColor));
	rmsLabel.setLabel(atext);
}
}







   void QPL_ScopeTrace::changeNDataSoll(int ds, double dt)
{
	NDataSoll=(int)ds;

    for (unsigned int i = 0; i< NDataSoll; i++)
    {
	//if (Scope->dt<=0.05)
        d_x[i] =dt * i;     // time axis
	    d_y[i] = offset;
    }
  	cData->setRawData(d_x, d_y, NDataSoll);
}



void QPL_ScopeTrace::show(bool v){

	visible=v;
	if (!visible) {
		cData->setStyle(QwtPlotCurve::NoCurve);
                sym.setStyle(QwtSymbol::NoSymbol); cData->setSymbol(sym);

	}else{	
                cData->setStyle(oldStyle);
                setSymbolStyle(oldSymbol);
	}
}

void QPL_ScopeTrace::setColor(const QColor& c)
{
		QPen pen;
		pen.setBrush(c);
		brush=QBrush(c);
		pen.setWidth(lineWidth);
		cData->setPen(pen);
		zeroAxis.setLinePen(QPen(brush,2.,Qt::DashDotLine));
		traceLabel.setLinePen(QPen(brush,2.,Qt::DashDotLine));
		QwtText ttext(traceName);
		ttext.setColor(QColor(c));
		traceLabel.setLabel(ttext);
}

void QPL_ScopeTrace::setWidth(int traceWidth)
{
		QPen pen;
		pen.setBrush(brush);
		pen.setWidth(traceWidth);
		lineWidth=traceWidth;
		cData->setPen(pen);
}

int  QPL_ScopeTrace::getWidth()
{
		return cData->pen().width();
}

   void QPL_ScopeTrace::setOffset(double o)
{
		//QRL_ScopeTrace[trace].offset=o;
   		 for (unsigned int i = 0; i< NDataSoll; i++)
  		  {
			d_y[i]=(((d_y[i]-offset)*dy))/dy+o;
			
		    }
	//cData[trace]->setRawData(d_x, ScopeData[trace].d_y, NDataSoll);
		offset=o;
		zeroAxis.setYValue(offset);
     

}

double  QPL_ScopeTrace::getOffset()
{
		return offset;
}

double QPL_ScopeTrace::getAverage()
{
average=0.;
	for (int k=0;k<NDataSoll;k++)
		average+=(d_y[k]-offset)*dy/((double)NDataSoll);

return average;


}

double QPL_ScopeTrace::getPP()
{
    min=(d_y[0]-offset)*dy;
        max=(d_y[0]-offset)*dy;
        for (unsigned int k=0;k<NDataSoll;k++){
                if (max<(d_y[k]-offset)*dy)
                        max=(d_y[k]-offset)*dy;
                if (min>(d_y[k]-offset)*dy)
                        min=(d_y[k]-offset)*dy;
        }
        PP=max-min;


return PP;


}
   void QPL_ScopeTrace::setDy(double d){

   		 for (unsigned int i = 0; i< NDataSoll; i++)
  		  {
			d_y[i]=(((d_y[i]-offset)*dy)/d+offset);
			
		    }
		dy=d;
		zeroAxis.setYValue(offset);
	

}

double  QPL_ScopeTrace::getDy()
{
		return dy;
}



   void QPL_ScopeTrace::showLabel(labelTypes lt){

  switch(lt){
    case lt_trace:traceLabel.show();
		  break;
    case lt_unit:
                unitLabel.setYValue(labelOffset-(0.5*labelCounter));
		labelCounter++;
		unitLabel.show();
		break;
  case lt_average:
              averageLabel.setYValue(labelOffset-(0.5*labelCounter));
		labelCounter++;
		averageLabel.show();
	      break;
  case lt_min:
              minLabel.setYValue(labelOffset-(0.5*labelCounter));
		labelCounter++;
		minLabel.show();
	      break;
   case lt_max:
                 maxLabel.setYValue(labelOffset-(0.5*labelCounter));
		labelCounter++;
		maxLabel.show();
		break;
  case lt_pp:
                ppLabel.setYValue(labelOffset-(0.5*labelCounter));
		labelCounter++;
		ppLabel.show();
		break;
  case lt_rms:
                rmsLabel.setYValue(labelOffset-(0.5*labelCounter));
		labelCounter++;
		rmsLabel.show();
		break;
}
		

}






   void QPL_ScopeTrace::hideLabel(labelTypes lt){




  switch(lt){
    case lt_trace:traceLabel.hide();
		  break;
    case lt_unit:
		labelCounter=0;
		unitLabel.hide();
		break;
  case lt_average:
		labelCounter=0;
		averageLabel.hide();
	      break;
  case lt_min:
		labelCounter=0;
		minLabel.hide();
	      break;
   case lt_max:
		labelCounter=0;
		maxLabel.hide();
		break;
  case lt_pp:
		labelCounter=0;
		ppLabel.hide();
		break;
  case lt_rms:
		labelCounter=0;
		rmsLabel.hide();
		break;
}



}
   bool QPL_ScopeTrace::isLabelVisible(labelTypes lt){
bool isvisible=false;
  switch(lt){
    case lt_trace:isvisible=traceLabel.isVisible();
		  break;
    case lt_unit:
		isvisible=unitLabel.isVisible();
		break;
  case lt_average:
		isvisible=averageLabel.isVisible();
	      break;
  case lt_min:
		isvisible=minLabel.isVisible();
	      break;
   case lt_max:
		isvisible=maxLabel.isVisible();
		break;
  case lt_pp:
		isvisible=ppLabel.isVisible();
		break;
  case lt_rms:
		isvisible=rmsLabel.isVisible();
		break;
}
return isvisible;




}
   void QPL_ScopeTrace::setLabelVisible(labelTypes lt, bool setvisible){
  switch(lt){
    case lt_trace:
		  traceLabel.setVisible(setvisible);
		  break;
    case lt_unit:
		unitLabel.setVisible(setvisible);
		break;
  case lt_average:
		averageLabel.setVisible(setvisible);
	      break;
  case lt_min:
		minLabel.setVisible(setvisible);
	      break;
   case lt_max:
		maxLabel.setVisible(setvisible);
		break;
  case lt_pp:
		ppLabel.setVisible(setvisible);
		break;
  case lt_rms:
		rmsLabel.setVisible(setvisible);
		break;
}




}






  void QPL_ScopeTrace::setLabelsXValue(double x){

		traceLabel.setXValue(x);
		unitLabel.setXValue(x);
		averageLabel.setXValue(x);
		minLabel.setXValue(x);
		maxLabel.setXValue(x);
		ppLabel.setXValue(x);
		rmsLabel.setXValue(x);

}

void QPL_ScopeTrace::setZeroAxis(bool b){

if (b)
zeroAxis.show();
else
zeroAxis.hide();




}





void QPL_ScopeTrace::setName(const QString &text){

	
	traceName=QString(text);
	
	QwtText ttext(traceName);
   	ttext.setColor(QColor(brush.color()));
    	traceLabel.setLabel(ttext);


}


   void QPL_ScopeTrace::setValue(int i, double v){
   index=i;
   
   d_y[index]=(v)/getDy()+getOffset();
  
   
   }

   void QPL_ScopeTrace::moveDataToRight(int time){
   
   
   		for (unsigned int i = NDataSoll - 1; i > time; i-- ){
        		d_y[i] = d_y[i-1-time];
		}
    index=index-1-time;
   
   }
   void QPL_ScopeTrace::moveDataToLeft(int time){
   
   		for (unsigned int i = 0; i < NDataSoll-time-1; i++ ){
		if ((i+time+1)<NDataSoll)
			d_y[i] = d_y[i+time+1];
		}
                index=index+time+1;
   }

QDataStream& operator<<(QDataStream &out, const QPL_ScopeTrace *d){
        qint32 a;
		out << d->traceName;
		out << d->zeroAxis.isVisible();
		out << d->averageLabel.isVisible();
		out << d->unitLabel.isVisible();
		out << d->minLabel.isVisible();
		out << d->maxLabel.isVisible();
		out << d->ppLabel.isVisible();
		out << d->rmsLabel.isVisible();

		out << d->traceLabel.isVisible();

		out << d->offset;
		out << d->dy;
		out << d->brush.color();
		a=d->lineWidth; out <<a;
		out << d->visible;

                out <<d->cData->style();
                out <<d->cData->symbol().pen().color();
                out <<d->cData->symbol().brush().color();
                out <<d->cData->symbol().style();
                out <<d->cData->symbol().size().width();

	return out;
}


QDataStream& operator>>(QDataStream &in, QPL_ScopeTrace(*d)){
	QSize s;QPoint p;bool b; QColor c; qint32 a;QFont f; double dd;
	QString str; 

	
// 		in >> str; d.setName( str);
// 		in >> b; d.setZeroAxis(b);
// 		in >> b; d.averageLabel.isVisible()(b);
// 		in >> b; d.setUnitLabel(b);
// 		in >> b; d.setMinLabel(b);
// 		in >> b; d.setMaxLabel(b);
// 		in >> b; d.setPPLabel(b);
// 		in >> b; d.setRMSLabel(b);
// 		in >> b; d.setLabel(b);
// 		in >> dd; d.setOffset(dd);
// 		in >> dd; d.setDy(dd);
// 		in >> c; d.setColor(c);
// 		in >> a; d.setWidth((int)a);
// 		in >> b; d.show(b);
		in >> str;
		in >> b; 
		in >> b; 
		in >> b; 
		in >> b; 
		in >> b; 
		in >> b; 
		in >> b; 
		in >> b; 
		in >> dd; 
		in >> dd; 
		in >> c;
		in >> a;
		in >> b;




	return in;
}
