/****************************************************************************
** Meta object code from reading C++ file 'tcp_event_client.h'
**
** Created: Wed Feb 8 17:58:00 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "tcp_event_client.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tcp_event_client.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_tcp_event_client[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   18,   17,   17, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_tcp_event_client[] = {
    "tcp_event_client\0\0value\0setPBValue(int)\0"
};

const QMetaObject tcp_event_client::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_tcp_event_client,
      qt_meta_data_tcp_event_client, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &tcp_event_client::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *tcp_event_client::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *tcp_event_client::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_tcp_event_client))
        return static_cast<void*>(const_cast< tcp_event_client*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int tcp_event_client::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: setPBValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE