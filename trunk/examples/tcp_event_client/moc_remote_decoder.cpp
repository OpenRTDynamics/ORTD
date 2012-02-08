/****************************************************************************
** Meta object code from reading C++ file 'remote_decoder.h'
**
** Created: Tue Feb 7 16:27:48 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "remote_decoder.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'remote_decoder.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rt_server_decoder[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      23,   19,   18,   18, 0x05,
      41,   18,   18,   18, 0x05,
      67,   61,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
     103,   87,   83,   18, 0x0a,
     135,   18,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rt_server_decoder[] = {
    "rt_server_decoder\0\0cmd\0newORTDCmd(char*)\0"
    "scopesInitialised()\0value\0newPBValue(int)\0"
    "int\0DataFromTcpLine\0feed_external_data(std::string)\0"
    "init_sending()\0"
};

const QMetaObject rt_server_decoder::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_rt_server_decoder,
      qt_meta_data_rt_server_decoder, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rt_server_decoder::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rt_server_decoder::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rt_server_decoder::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rt_server_decoder))
        return static_cast<void*>(const_cast< rt_server_decoder*>(this));
    return QObject::qt_metacast(_clname);
}

int rt_server_decoder::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: newORTDCmd((*reinterpret_cast< char*(*)>(_a[1]))); break;
        case 1: scopesInitialised(); break;
        case 2: newPBValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: { int _r = feed_external_data((*reinterpret_cast< std::string(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 4: init_sending(); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void rt_server_decoder::newORTDCmd(char * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void rt_server_decoder::scopesInitialised()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void rt_server_decoder::newPBValue(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
