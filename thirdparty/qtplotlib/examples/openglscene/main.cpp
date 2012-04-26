#include "openglscene.h"

#include <QtGui>
#include <QGLWidget>
#include "qwt_counter.h"
#include "data_plot.h"
#include "scope.h"

class GraphicsView : public QGraphicsView
{
public:
    GraphicsView()
    {
        setWindowTitle(tr("3D Model Viewer"));
    }

protected:
    void resizeEvent(QResizeEvent *event) {
        if (scene())
            scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
        QGraphicsView::resizeEvent(event);
    }
};



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
     		
	DataPlot *data = new DataPlot(this,plotdata,plot);

        connect(counter, SIGNAL(valueChanged(double)),
            data, SLOT(setTimerInterval(double)) );

        counter->setValue(20.0);

	GraphicsView *view = new GraphicsView();
        QGLWidget *glWidget = new QGLWidget(QGLFormat(QGL::SampleBuffers));
        //QGLWidget *glWidget = new QGLWidget(QGLFormat(QGL::DoubleBuffer));
	view->setViewport(glWidget);
//	view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
        view->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
	OpenGLScene* scene=new OpenGLScene(plot);
	view->setScene(scene);
	view->show();
	
//QWidget * centralwidget = new QWidget ( 0 );
this->setCentralWidget ( view );
// QHBoxLayout * outerLayout = new QHBoxLayout ( centralwidget );
// outerLayout->setSpacing ( 0 );
// outerLayout->setMargin ( 0 );

//outerLayout->insertWidget ( 0, view, 1 );

//         DataPlot *plot = new DataPlot(this);
//         setCentralWidget(plot);
// 

    }
};



int main(int argc, char **argv)
{
    QApplication app(argc, argv);

        MainWindow mainWindow;
#if QT_VERSION < 0x040000
    app.setMainWidget(&mainWindow);
#endif

    mainWindow.resize(1024,768);
    mainWindow.show();
    


   // view.resize(1024, 768);

    return app.exec();
}
