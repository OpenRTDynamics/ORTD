#ifndef QT_EVENT_COMMUNICATION_H
#define QT_EVENT_COMMUNICATION_H

#include <QObject>
#include <stdlib.h>
#include "remote_decoder.h"

class rt_server_decoder;

class qt_event_communication : public QObject
{
    Q_OBJECT

    public:
        qt_event_communication(char *hostname, int port);
        ~qt_event_communication();
        bool emit_cmd(char *cmd); // emits the command as signal - returns ALWAYSS true

    public slots:
        void handle_data(std::string DataFromTcpLine); // if new data from the tcp server is received this function is called
        void init_sending(); // if the tcp client is connected this function is called
        // activated when the scopes are initialised
        void test_function();

    signals:
        // this signal is used to write data to the tcp stream
        void newORTDCmd(char *cmd);
        // this signal is sended after the scope initialisation
        void scopesInitialised();
        // this signal sets the value of the progress bar
        void newPBValue(int value);

    private:
        rt_server_decoder *rt_server_dec;
};

#endif // QT_EVENT_COMMUNICATION_H
