
#include "qt_event_communication.h"

#define FAILURE -1
#define NOTHING_HAPPENED 0
#define MORE_SCOPES_TO_READ 1
#define MORE_SCOPES_INFOS_TO_READ 2
#define ALL_SCOPES_WITH_INFOS_READ 3
#define SCOPE_DATA_ARRIVED 4

qt_event_communication::qt_event_communication(char *hostname, int port)
{
    rt_server_dec = new rt_server_decoder(hostname, port, this);
}

qt_event_communication::~qt_event_communication()
{
    delete this->rt_server_dec;
}

bool qt_event_communication::emit_cmd(char *cmd)
{
    emit newORTDCmd(cmd);
    return true;
}

void qt_event_communication::handle_data(std::string DataFromTcpLine)
{
    int data_state;
    data_state = rt_server_dec->feed_external_data(DataFromTcpLine);
    //printf("feed data\n");
    if (data_state == ALL_SCOPES_WITH_INFOS_READ)
    {
        emit scopesInitialised();
    }
    else if (data_state == SCOPE_DATA_ARRIVED)
    {
        rt_server_decoder_scope *changed_sc = rt_server_dec->GetTheScopeThatWasChanged();
        if (changed_sc != NULL) {
            double data[1000];

            changed_sc->read_data(data);
            //printf("v = %f, %f, %f, %f, %f\n", data[0] / 100, data[1], data[2] / 1000, data[3], data[4] / 4);

            double ref = data[0] / 100;
            double y = data[1];
            double u = data[2] / 1000;
            double angle = data[3];
            double force = data[4] / 4;

            scope_ref_data->setScopeValue(ref, 0);
            scope_ref_data->setScopeValue(y, 1);
            ui->scope_ref->setValue(scope_ref_data->getScopeValue());

            scope_actuating_data->setScopeValue(u, 0);
            ui->scope_actuating->setValue(scope_actuating_data->getScopeValue());

            scope_angle_data->setScopeValue(angle, 0);
            ui->scope_angle->setValue(scope_angle_data->getScopeValue());

        }
    }
}

void qt_event_communication::init_sending()
{
    rt_server_dec->init_sending();
}

void qt_event_communication::run()
{
    std::string name("gui_interface");
    rt_server_decoder_scope *sc = rt_server_dec->get_scope_by_name(name);
    if (sc == NULL) {
      printf("scope %s not found\n", name.c_str() );

      //delete rt_server_dec;
      exit(0);
    }

    sc->start_stream();

}

void qt_event_communication::set_ui(Ui::GuiInterface *ui)
{
    this->ui = ui;
}

