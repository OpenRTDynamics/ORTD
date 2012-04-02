#include "guiinterface.h"
#include "ui_guiinterface.h"

GuiInterface::GuiInterface(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GuiInterface)
{
    ui->setupUi(this);

    //
    // ADAPT: Configure your data source
    //

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
    connect(qt_event_com, SIGNAL(scopesInitialised()), qt_event_com, SLOT(run()));

    //
    // Set-up Scopes (ADAPT)
    //

    qt_event_com->scope_ref_data = new QPL_ScopeData(2,0.01,tr("1"),QStringList("1") << "2");  // scope with two traces; add more << "n" as you increase traces
    ui->scope_ref->initTraces(qt_event_com->scope_ref_data,0);
    //ui->scope_ref->trace(0)->setDy(2);
    //ui->scope_ref->trace(1)->setDy(2);
    ui->scope_ref->setAxisScale(0,0,10,1);
    ui->scope_ref->startRefresh();

//    qt_event_com->scope_actuating_data = new QPL_ScopeData(1,0.01,tr("1"),QStringList("1")); // scope with only one trace
//    ui->scope_actuating->initTraces(qt_event_com->scope_actuating_data,0);
//    //ui->scope_actuating->trace(0)->setDy(0.2);
//    ui->scope_actuating->setAxisScale(0,0,1,0.1);
//    ui->scope_actuating->startRefresh();

//    qt_event_com->scope_angle_data = new QPL_ScopeData(1,0.01,tr("1"),QStringList("1"));
//    ui->scope_angle->initTraces(qt_event_com->scope_angle_data,0);
//    //ui->scope_angle->trace(0)->setDy(36);
//    ui->scope_angle->setAxisScale(0,0,180,10);
//    ui->scope_angle->startRefresh();


    qt_event_com->set_ui( ui );

}

GuiInterface::~GuiInterface()
{
    delete ui;
    delete qt_event_com;
    delete tcpStream;
}

void GuiInterface::changeEvent(QEvent *e)
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

void GuiInterface::on_switch_schematic_currentRowChanged(int currentRow)
{
    printf("set_param switch_schematic # %d\n", currentRow);

    char tmp[200];

    sprintf(tmp, "set_param switch_schematic # %d \n", currentRow+1);
    qt_event_com->emit_cmd(tmp);
}

//void GuiInterface::on_parameter_xx_valueChanged(double val) // assign this to a double spin box slot
//{
//    char tmp[200];

//    setlocale(LC_NUMERIC,"C");         // C-Standard

//    sprintf(tmp, "set_param eemgcntrl_kg # %f \n", val);
//    qt_event_com->emit_cmd(tmp);
//}

