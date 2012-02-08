
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
            printf("v = %f, %f\n", data[0], data[5] );
            emit newPBValue(data[0]);               // Change here for other widgets than the processbar
        }
    }
}

void qt_event_communication::init_sending()
{
    rt_server_dec->init_sending();
}

void qt_event_communication::test_function()
{
    std::string name("osc_output");
    rt_server_decoder_scope *sc = rt_server_dec->get_scope_by_name(name);
    if (sc == NULL) {
      printf("scope %s not found\n", name.c_str() );

      //delete rt_server_dec;
      exit(0);
    }

    sc->start_stream();

}
