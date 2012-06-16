#include "tcpclient.h"
#include <QDebug>
#include <stdio.h>
#include <iostream>
#include <sstream>


tcpClient::tcpClient(char* hostname, int port)
{
    // instantiate the tcp socket
    socket = new QTcpSocket(this);

    // readyRead gets called if the socket receives data
    // connected gets called if the connection to the tcp server has been established
    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

    socket->connectToHost(QString(hostname), port);
    qDebug() << "nach connectToHost" << endl;
    setWindowTitle( "TCP-Client OpenRTDynamics" );
}

// write line to tell the tcp server something, like what to stream
void tcpClient::writeln_to_socket(char *line)
{
    socket->write(line);
}

// this function gets called when the socket received data from the tcp server
void tcpClient::readyRead()
{
    // read
    while(socket->canReadLine())
    {
        // read the received line
        QString line = socket->readLine().trimmed();
        std::string input_line = line.toStdString();

        // emit signal to refresh the Data
        emit newORTDData(input_line);

    }
}

// this function gets called if the connection to the tcp server is established
void tcpClient::connected()
{
    // emit signal that the client is connected
    emit clientConnected();

}
