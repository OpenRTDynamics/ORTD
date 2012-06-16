# -------------------------------------------------
# Project created by QtCreator 2012-03-01T16:05:12
# -------------------------------------------------
QT += network
TARGET = gui_interface
TEMPLATE = app
LIBS += -lqwt-qt4
INCLUDEPATH += /usr/include/qwt-qt4
unix|win32:LIBS += -lqpl
INCLUDEPATH += /usr/local/include
SOURCES += main.cpp \
    guiinterface.cpp \
    tcpclient.cpp \
    remote_decoder.cpp \
    qt_event_communication.cpp
HEADERS += guiinterface.h \
    tcpclient.h \
    remote_decoder.h \
    qt_event_communication.h
FORMS += guiinterface.ui
