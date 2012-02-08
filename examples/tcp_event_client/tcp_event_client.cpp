#include "tcp_event_client.h"
#include "ui_tcp_event_client.h"


tcp_event_client::tcp_event_client(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::tcp_event_client)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);

    qt_event_com = new qt_event_communication("localhost", 10000);
    // try to connect to localhost on port 10000 (OpenRTDynamics TCP-Server)
    tcpStream = new tcpClient("127.0.0.1", 10000);

    // connect the ORTDData signal with the refresh function
    connect(tcpStream, SIGNAL(newORTDData(std::string)), qt_event_com, SLOT(handle_data(std::string)));

    // connect the ORTDCmd signal with the write function
    connect(qt_event_com, SIGNAL(newORTDCmd(char*)), tcpStream, SLOT(writeln_to_socket(char*)));

    // connect the Connected signal with the initialize function
    connect(tcpStream, SIGNAL(clientConnected()), qt_event_com, SLOT(init_sending()));

    // connect the Initialised signal with the main_function
    connect(qt_event_com, SIGNAL(scopesInitialised()), qt_event_com, SLOT(test_function()));

    // connect the new progress bar value signal with the set function
    connect(qt_event_com, SIGNAL(newPBValue(int)), this, SLOT(setPBValue(int)));

}

void tcp_event_client::setPBValue(int value)
{
    ui->progressBar->setValue(value/4);
}

tcp_event_client::~tcp_event_client()
{
    delete ui;
}

void tcp_event_client::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
