#ifndef _DATA_PLOT_H
#define _DATA_PLOT_H 1

#include <qwt_plot.h>
#include "scope_data.h"
#include "scope.h"

const int PLOT_SIZE = 201;      // 0 to 200

class DataPlot  : public QObject
{
	Q_OBJECT
public:
    DataPlot(QObject *parent = 0,QPL_ScopeData *d=0,QPL_Scope *s =0);

public slots:
    void setTimerInterval(double interval);
  signals:
    void doUpdate();

protected:
    void timerEvent(QTimerEvent *e);

private:

    double d_y;
    double d_z; 
    QPL_ScopeData *data;
    QPL_Scope *scope;
    int d_interval; // timer in ms
    int d_timerId;
};

#endif
