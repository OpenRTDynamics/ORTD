#ifndef QT_EVENT_COMMUNICATION_H
#define QT_EVENT_COMMUNICATION_H

#include <QObject>
#include <stdlib.h>
#include "remote_decoder.h"
#include "qwt_plot.h"
#include "scope.h"
#include "ui_guiinterface.h"

namespace Ui {
    class GuiInterface;
}

class rt_server_decoder;

class qt_event_communication : public QObject
{
    Q_OBJECT

    public:
        qt_event_communication(char *hostname, int port);
        ~qt_event_communication();
        bool emit_cmd(char *cmd); // emits the command as signal - returns ALWAYSS true
        void set_ui(Ui::GuiInterface *ui);

    public slots:
        void handle_data(std::string DataFromTcpLine); // if new data from the tcp server is received this function is called
        void init_sending(); // if the tcp client is connected this function is called
        // activated when the scopes are initialised
        void run();

    signals:
        // this signal is used to write data to the tcp stream
        void newORTDCmd(char *cmd);
        // this signal is sended after the scope initialisation
        void scopesInitialised();

    private:
        rt_server_decoder *rt_server_dec;
        Ui::GuiInterface *ui;

    public:
        // scopes
        QPL_ScopeData *scope_ref_data;
        QPL_ScopeData *scope_actuating_data;
        QPL_ScopeData *scope_angle_data;
        QPL_ScopeData *scope_force_data;
};

#endif // QT_EVENT_COMMUNICATION_H
