#include <qapplication.h>
#include <qmainwindow.h>
#include <qwt_counter.h>
#include <qtoolbar.h>
#include <qlabel.h>
#include <qlayout.h>
#include "data_plot.h"
#include <scope.h>

class MainWindow: public QMainWindow
{
public:
    MainWindow()
    {
        QToolBar *toolBar = new QToolBar(this);
        toolBar->setFixedHeight(80);

#if QT_VERSION < 0x040000
        setDockEnabled(TornOff, true);
        setRightJustification(true);
#else
        toolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
#endif
        QWidget *hBox = new QWidget(toolBar);
        QLabel *label = new QLabel("Timer Interval", hBox);
        QwtCounter *counter = new QwtCounter(hBox);
        counter->setRange(-1.0, 100.0, 1.0);

        QHBoxLayout *layout = new QHBoxLayout(hBox);
        layout->addWidget(label);
        layout->addWidget(counter);
        layout->addWidget(new QWidget(hBox), 10); // spacer);

#if QT_VERSION >= 0x040000
        toolBar->addWidget(hBox);
#endif
        addToolBar(toolBar);

	QPL_ScopeData *plotdata = new QPL_ScopeData(1,0.001,tr("1"),QStringList("1"));
        QPL_Scope *plot = new QPL_Scope(this);
	plot->initTraces(plotdata,0);
	plot->trace(0)->setDy(0.2);
        plot->startRefresh();
        setCentralWidget(plot);
		
	DataPlot *data = new DataPlot(this,plotdata,plot);

        connect(counter, SIGNAL(valueChanged(double)),
            data, SLOT(setTimerInterval(double)) );

        counter->setValue(20.0);
    }
};

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    MainWindow mainWindow;
#if QT_VERSION < 0x040000
    a.setMainWidget(&mainWindow);
#endif

    mainWindow.resize(600,400);
    mainWindow.show();

    return a.exec(); 
}
