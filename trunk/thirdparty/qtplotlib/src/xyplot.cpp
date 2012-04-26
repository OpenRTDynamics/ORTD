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

#include "xyplot.h"


#include <stdlib.h>
class Zoomer: public QwtPlotZoomer
{
public:
    Zoomer(int xAxis, int yAxis, QwtPlotCanvas *canvas):
        QwtPlotZoomer(xAxis, yAxis, canvas)
    {
        setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);
        setTrackerMode(QwtPicker::AlwaysOff);
        setRubberBand(QwtPicker::NoRubberBand);

        // RightButton: zoom out by 1
        // Ctrl+RightButton: zoom out to full size

#if QT_VERSION < 0x040000
        setMousePattern(QwtEventPattern::MouseSelect2,
            Qt::RightButton, Qt::ControlButton);
#else
        setMousePattern(QwtEventPattern::MouseSelect2,
            Qt::RightButton, Qt::ControlModifier);
#endif
        setMousePattern(QwtEventPattern::MouseSelect3,
            Qt::RightButton);
    }
};



QPL_XYPlot::QPL_XYPlot(QWidget *parent)
        :QwtPlot(parent)
{

    this->setObjectName(QString::fromUtf8("Scope"));
#if QT_VERSION >= 0x040000
    #ifdef Q_WS_X11
    /*
       Qt::WA_PaintOnScreen is only supported for X11, but leads
       to substantial bugs with Qt 4.2.x/Windows
     */
    //this->canvas()->setAttribute(Qt::WA_PaintOnScreen, true);
    #endif
    #endif
        plotting=true;

        yMajorTicks=10;dy=.1; yOffset=0.;
        ymin=yOffset-0.5*(yMajorTicks*dy);
       ymax=yOffset+0.5*(yMajorTicks*dy);
      yStep=(ymax-ymin)/yMajorTicks;
        MaxDataPoints=10000;
        dx=.1;
        xMajorTicks=10;
        xmin=0;
       xmax=(xMajorTicks*dx);
        Divider=1;
         xStep=(xmax-xmin)/xMajorTicks;
        Ncurve=0;
        gridColor=Qt::blue;

        bgColor=QColor(240,240,240);
    picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
        QwtPicker::PointSelection | QwtPicker::DragSelection,
        QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
        this->canvas());
    picker->setRubberBandPen(QColor(gridColor));
    picker->setRubberBand(QwtPicker::CrossRubberBand);
    picker->setTrackerPen(QColor(gridColor));

    zeroLine = new QwtPlotMarker();
    //zeroLine->setLabel(QString::fromLatin1("y = 0"));
    zeroLine->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
    zeroLine->setLineStyle(QwtPlotMarker::HLine);
    zeroLine->setYValue(0.0);
    zeroLine->setLinePen(QPen(QColor(gridColor),1,Qt::DotLine));
    zeroLine->attach(this);

    vertLine = new QwtPlotMarker();
    //zeroLine->setLabel(QString::fromLatin1("y = 0"));
    vertLine->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
    vertLine->setLineStyle(QwtPlotMarker::VLine);
    vertLine->setXValue(xmax/2.);
    vertLine->setLinePen(QPen(QColor(gridColor),1,Qt::DotLine));
    vertLine->attach(this);

    zoomer[0] = new Zoomer( QwtPlot::xBottom, QwtPlot::yLeft,this->canvas());
    zoomer[0]->setRubberBand(QwtPicker::RectRubberBand);
    zoomer[0]->setRubberBandPen(QColor(gridColor));
    zoomer[0]->setTrackerMode(QwtPicker::ActiveOnly);
    zoomer[0]->setTrackerPen(QColor(gridColor));

    zoomer[1] = new Zoomer(QwtPlot::xTop, QwtPlot::yRight,
         this->canvas());

    panner = new QwtPlotPanner(this->canvas());
    panner->setMouseButton(Qt::MidButton);
    panner->setEnabled(false);

    zoomer[0]->setEnabled(false);
    zoomer[0]->zoom(0);

    zoomer[1]->setEnabled(false);
    zoomer[1]->zoom(0);

    zoomer[0]->setZoomBase(false);
    zoomer[1]->setZoomBase(false);


    bottomText = new QwtPlotMarker();
    QwtText bt(tr("x: %1.%2 sec/dev  roll <-").arg(0).arg(1));
    bt.setColor(QColor(gridColor));
    bottomText->setLabel(bt);
    bottomText->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
    bottomText->setLineStyle(QwtPlotMarker::NoLine);
    bottomText->setValue(0.0,-5.);
   bottomText->attach(this);

   Traces=NULL;

       grid = new QwtPlotGrid;
    //grid->enableXMin(true);
    grid->setMajPen(QPen(gridColor, 0, Qt::DotLine));
    grid->attach(this);

           //  qwtPlot->setTitle(tr(Scope->name));
       //qwtPlot->setAxisTitle(QwtPlot::xBottom, "Delta Time [sec]");
       this->setAxisScale(QwtPlot::xBottom, xmin, xmax,xStep);
        //qwtPlot->axisScaleDiv(QwtPlot::xBottom)=new QwtScaleDiv(1,0);
       // qwtPlot->setAxisTitle(QwtPlot::yLeft, "Values");
       this->setAxisScale(QwtPlot::yLeft, ymin, ymax,yStep);
           this->enableAxis(QwtPlot::xBottom,false);
       this->enableAxis(QwtPlot::yLeft,false);
          replot();
}


void QPL_XYPlot::initTraces(int ncurve, int nsoll)
{


    //picker->setEnabled(!on);


       NDataSoll=nsoll;

        RefreshRate=20.;


       Ncurve=ncurve;
//if (Ncurve>0){
       QPen pen;
        Traces = new QPL_XYPlotTrace*[Ncurve];


       for (unsigned int j=0;j<Ncurve;j++){
                Traces[j] = new QPL_XYPlotTrace(this, MaxDataPoints, j);
                Traces[j]->changeNDataSoll(NDataSoll);
                Traces[j]->setGridColor(gridColor);
                Traces[j]->setLineStyle(QwtPlotCurve::Dots);
                //Traces[j]->setName(Scope->getTraceNames().at(j));
                #if QT_VERSION >= 0x040000
                //to slow
                 //cData[j]->setRenderHint(QwtPlotItem::RenderAntialiased);
                #endif

        }

  //  }
           // grid
//        timer = new QTimer(this);
//        connect(timer, SIGNAL(timeout()), this, SLOT(refresh()));
//        timer->start((int)(1./RefreshRate*1000.));

}



QPL_XYPlot::~QPL_XYPlot(){

    if (Traces) {
    for (unsigned int j=0;j<Ncurve;j++){
            delete Traces[j];
    }
    delete[] Traces;
    }
}


void QPL_XYPlot::refresh()
{
        this->replot();

}


   void QPL_XYPlot::changeDataPoints(double dp)
{
        //if (dp<(1/((xmax-xmin)/dp)/100))
        //	return;
      // timer->stop();
        NDataSoll=(int)dp;


     for (unsigned int j=0;j<Ncurve;j++){
        Traces[j]->changeNDataSoll(NDataSoll);
     }
 //    timer->start((int)(1./RefreshRate*1000.));



}

void QPL_XYPlot::setValue(const QVector< QVector< QVector<float> > >&v){
    for (int j=0;j<v.size();j++){
        for (int i=0;i<Ncurve;i++){
            trace(i)->moveDataToRight(1);
            trace(i)->setValue(0,v.at(j).at(i).at(0),v.at(j).at(i).at(1));
    //        for (int j=0;j<v.at(i).size();j++){
    ////             printf(" %f ",v.at(i).at(j));
    //             (matrixPlot->item(i,j))->setText(tr("%1").arg(v.at(i).at(j)));
    //         }
    ////         printf("\n");
        }
    }
    this->replot();
}


QDataStream& operator<<(QDataStream &out, const QPL_XYPlot &d){
        qint32 a;
      //  out  << d.size()  << d.pos() << d.isVisible();
        a=d.Ncurve; out << a;
        a=d.NDataSoll;out << a;
        for (unsigned int nn=0; nn<d.Ncurve;++nn){

                out << *(d.Traces[nn]);
        }
    }


QDataStream& operator>>(QDataStream &in, QPL_XYPlot(&d)){
    QSize s;QPoint p;bool b; QColor c; qint32 a,a2;QFont f; double dd;
    QString str; int Ncurve;
//    in >> s;d.resize(s);
//    in >> p; d.move(p);
//    in >> b; d.setVisible(b);
    in >> a; Ncurve=(int)a;
    in >> a; d.changeDataPoints(a);
     for (int nn=0; nn<Ncurve;++nn){
       if (nn<d.Ncurve) {
            in >> *(d.Traces[nn]);
       }
   }
}
