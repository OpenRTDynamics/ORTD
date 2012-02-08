#include <QtGui/QApplication>
#include <iostream>


#include "tcp_event_client.h"



using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    tcp_event_client w;
    cout << "starting ..." << endl;

    w.show();

    return a.exec();
}
