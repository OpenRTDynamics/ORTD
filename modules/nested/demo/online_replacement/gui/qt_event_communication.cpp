
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

            double val = data[0];


            scope_ref_data->setScopeValue(val, 0);
            //scope_ref_data->setScopeValue(y, 1);
            ui->scope_ref->setValue(scope_ref_data->getScopeValue());

        }
    }
}

void qt_event_communication::init_sending()
{
    rt_server_dec->init_sending();
}

void qt_event_communication::run()
{
    std::string name("ControlOutput"); // the name of the scope
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

//void GuiInterface::on_parameter_xx_valueChanged(double val) // assign this to a double spin box slot
//{
//    char tmp[200];

//    setlocale(LC_NUMERIC,"C");         // C-Standard

//    sprintf(tmp, "set_param eemgcntrl_kg # %f \n", val);
//    qt_event_com->emit_cmd(tmp);
//}

