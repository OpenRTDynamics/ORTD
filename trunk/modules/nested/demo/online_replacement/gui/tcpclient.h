#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QMainWindow>
#include <QTcpSocket>

class tcpClient : public QMainWindow
{
    Q_OBJECT

    public:
        tcpClient(char *hostname, int port);

    public slots:
        // write line to tell the tcp server something, like what to stream
        void writeln_to_socket(char *line);

    private slots:
        // this function gets called if the socket received data from the server
        void readyRead();

        // this function gets called when the connection to the server is established
        void connected();

    signals:
        // this signal is used to refresh the Data from the tcp server
        void newORTDData(std::string DataFromTcpLine);
        // this signal shows that the client is connected
        void clientConnected();

    private:
        // tcp socket
        QTcpSocket *socket;
        // variables to store data from the tcp server

};

#endif // TCPCLIENT_H
