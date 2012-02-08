/********************************************************************************
** Form generated from reading UI file 'tcp_event_client.ui'
**
** Created: Tue Feb 7 16:27:44 2012
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TCP_EVENT_CLIENT_H
#define UI_TCP_EVENT_CLIENT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QProgressBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_tcp_event_client
{
public:
    QWidget *centralWidget;
    QProgressBar *progressBar;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *tcp_event_client)
    {
        if (tcp_event_client->objectName().isEmpty())
            tcp_event_client->setObjectName(QString::fromUtf8("tcp_event_client"));
        tcp_event_client->resize(600, 400);
        centralWidget = new QWidget(tcp_event_client);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        progressBar = new QProgressBar(centralWidget);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setGeometry(QRect(150, 140, 118, 23));
        progressBar->setValue(24);
        tcp_event_client->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(tcp_event_client);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 600, 23));
        tcp_event_client->setMenuBar(menuBar);
        mainToolBar = new QToolBar(tcp_event_client);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        tcp_event_client->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(tcp_event_client);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        tcp_event_client->setStatusBar(statusBar);

        retranslateUi(tcp_event_client);

        QMetaObject::connectSlotsByName(tcp_event_client);
    } // setupUi

    void retranslateUi(QMainWindow *tcp_event_client)
    {
        tcp_event_client->setWindowTitle(QApplication::translate("tcp_event_client", "tcp_event_client", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class tcp_event_client: public Ui_tcp_event_client {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TCP_EVENT_CLIENT_H
