/****************************************************************************
** Meta object code from reading C++ file 'tcpclient.h'
**
** Created: Wed Feb 22 15:35:22 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "tcpclient.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tcpclient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_tcpClient[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      27,   11,   10,   10, 0x05,
      52,   10,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
      75,   70,   10,   10, 0x0a,
     100,   10,   10,   10, 0x08,
     112,   10,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_tcpClient[] = {
    "tcpClient\0\0DataFromTcpLine\0"
    "newORTDData(std::string)\0clientConnected()\0"
    "line\0writeln_to_socket(char*)\0readyRead()\0"
    "connected()\0"
};

const QMetaObject tcpClient::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_tcpClient,
      qt_meta_data_tcpClient, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &tcpClient::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *tcpClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *tcpClient::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_tcpClient))
        return static_cast<void*>(const_cast< tcpClient*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int tcpClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: newORTDData((*reinterpret_cast< std::string(*)>(_a[1]))); break;
        case 1: clientConnected(); break;
        case 2: writeln_to_socket((*reinterpret_cast< char*(*)>(_a[1]))); break;
        case 3: readyRead(); break;
        case 4: connected(); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void tcpClient::newORTDData(std::string _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void tcpClient::clientConnected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
