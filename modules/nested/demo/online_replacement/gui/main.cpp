#include <QtGui/QApplication>
#include <iostream>
#include "guiinterface.h"

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GuiInterface w;
    cout << "starting ..." << endl;
    w.show();
    return a.exec();
}
