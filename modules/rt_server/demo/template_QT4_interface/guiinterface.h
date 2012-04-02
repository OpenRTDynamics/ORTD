#ifndef GUIINTERFACE_H
#define GUIINTERFACE_H

#include <QMainWindow>
#include "tcpclient.h"
#include "qt_event_communication.h"

namespace Ui {
    class GuiInterface;
}

class GuiInterface : public QMainWindow {
    Q_OBJECT
public:
    GuiInterface(QWidget *parent = 0);
    ~GuiInterface();


protected:
    void changeEvent(QEvent *e);

private slots:
    void on_switch_schematic_currentRowChanged(int currentRow);

private:
    Ui::GuiInterface *ui;
    tcpClient *tcpStream;

    qt_event_communication *qt_event_com;
};

#endif // GUIINTERFACE_H
