#ifndef OPENGLSCENE_H
#define OPENGLSCENE_H



#include <QGraphicsScene>
#include <QLabel>
#include <QTime>
#include "data_plot.h"
#include "scope.h"

#ifndef QT_NO_CONCURRENT
#include <QFutureWatcher>
#endif

class Model;

class OpenGLScene : public QGraphicsScene
{
    Q_OBJECT

public:
    OpenGLScene(QPL_Scope *plot);

    void drawBackground(QPainter *painter, const QRectF &rect);

public slots:
    void setBackgroundColor();
  
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void wheelEvent(QGraphicsSceneWheelEvent * wheelEvent);

private:
    QDialog *createDialog(const QString &windowTitle) const;

   


    QColor m_backgroundColor;

  

    QTime m_time;
    int m_lastTime;
    int m_mouseEventTime;

   QTimer *scopeTimer;


#ifndef QT_NO_CONCURRENT
    QFutureWatcher<Model *> m_modelLoader;
#endif
};

#endif
