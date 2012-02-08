#ifndef TCP_EVENT_CLIENT_H
#define TCP_EVENT_CLIENT_H

#include <QMainWindow>
#include "tcpclient.h"
#include "qt_event_communication.h"

namespace Ui {
    class tcp_event_client;
}

class tcp_event_client : public QMainWindow {
    Q_OBJECT
public:
    tcp_event_client(QWidget *parent = 0);
    ~tcp_event_client();

public slots:
    // set the value of the progress bar
    void setPBValue(int value);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::tcp_event_client *ui;
    tcpClient *tcpStream;

    qt_event_communication *qt_event_com;
};

#endif // TCP_EVENT_CLIENT_H
