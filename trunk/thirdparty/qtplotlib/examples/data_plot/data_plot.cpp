#include <stdlib.h>
#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>
#include <qwt_math.h>
#include "data_plot.h"

//
//  Initialize main window
//
DataPlot::DataPlot(QObject *parent, QPL_ScopeData *d, QPL_Scope *s):
    QObject(parent),
    data(d),
    scope(s),
    d_interval(0),
    d_timerId(-1)
{
        d_y = 0; // time axis
        d_z = 0;

    setTimerInterval(0.0);
}

void DataPlot::setTimerInterval(double ms)
{
    d_interval = qRound(ms);

    if ( d_timerId >= 0 )
    {
        killTimer(d_timerId);
        d_timerId = -1;
    }
    if (d_interval >= 0 )
        d_timerId = startTimer(d_interval);
}

//  Generate new values 
void DataPlot::timerEvent(QTimerEvent *)
{
    static double phase = 0.0;

    if (phase > (M_PI - 0.0001)) 
        phase = 0.0;

    d_y = sin(phase) * (-1.0 + 2.0 * double(rand()) / double(RAND_MAX));
    data->setScopeValue(d_y,0);
    scope->setValue(data->getScopeValue());
    d_z = 0.8 - (2.0 * phase/M_PI) + 0.4 * double(rand()) / double(RAND_MAX);


    phase += M_PI * 0.002;
}
