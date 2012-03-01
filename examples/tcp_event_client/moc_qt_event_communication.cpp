/****************************************************************************
** Meta object code from reading C++ file 'qt_event_communication.h'
**
** Created: Wed Feb 22 15:35:23 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "qt_event_communication.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qt_event_communication.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_qt_event_communication[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      28,   24,   23,   23, 0x05,
      46,   23,   23,   23, 0x05,
      72,   66,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
     104,   88,   23,   23, 0x0a,
     129,   23,   23,   23, 0x0a,
     144,   23,   23,   23, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_qt_event_communication[] = {
    "qt_event_communication\0\0cmd\0"
    "newORTDCmd(char*)\0scopesInitialised()\0"
    "value\0newPBValue(int)\0DataFromTcpLine\0"
    "handle_data(std::string)\0init_sending()\0"
    "test_function()\0"
};

const QMetaObject qt_event_communication::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_qt_event_communication,
      qt_meta_data_qt_event_communication, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &qt_event_communication::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *qt_event_communication::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *qt_event_communication::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_qt_event_communication))
        return static_cast<void*>(const_cast< qt_event_communication*>(this));
    return QObject::qt_metacast(_clname);
}

int qt_event_communication::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: newORTDCmd((*reinterpret_cast< char*(*)>(_a[1]))); break;
        case 1: scopesInitialised(); break;
        case 2: newPBValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: handle_data((*reinterpret_cast< std::string(*)>(_a[1]))); break;
        case 4: init_sending(); break;
        case 5: test_function(); break;
        default: ;
        }
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void qt_event_communication::newORTDCmd(char * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void qt_event_communication::scopesInitialised()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void qt_event_communication::newPBValue(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
