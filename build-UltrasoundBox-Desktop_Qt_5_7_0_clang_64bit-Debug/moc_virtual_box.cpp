/****************************************************************************
** Meta object code from reading C++ file 'virtual_box.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../UltrasoundBox/virtual_box.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'virtual_box.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_RenderItem_t {
    QByteArrayData data[11];
    char stringdata0[77];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_RenderItem_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_RenderItem_t qt_meta_stringdata_RenderItem = {
    {
QT_MOC_LITERAL(0, 0, 10), // "RenderItem"
QT_MOC_LITERAL(1, 11, 2), // "cc"
QT_MOC_LITERAL(2, 14, 0), // ""
QT_MOC_LITERAL(3, 15, 7), // "addData"
QT_MOC_LITERAL(4, 23, 14), // "unsigned char*"
QT_MOC_LITERAL(5, 38, 9), // "setParams"
QT_MOC_LITERAL(6, 48, 8), // "QDomNode"
QT_MOC_LITERAL(7, 57, 4), // "stop"
QT_MOC_LITERAL(8, 62, 5), // "ready"
QT_MOC_LITERAL(9, 68, 4), // "addD"
QT_MOC_LITERAL(10, 73, 3) // "sss"

    },
    "RenderItem\0cc\0\0addData\0unsigned char*\0"
    "setParams\0QDomNode\0stop\0ready\0addD\0"
    "sss"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_RenderItem[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x06 /* Public */,
       3,    3,   50,    2, 0x06 /* Public */,
       5,    1,   57,    2, 0x06 /* Public */,
       7,    0,   60,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    0,   61,    2, 0x0a /* Public */,
       9,    0,   62,    2, 0x0a /* Public */,
      10,    0,   63,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 4, QMetaType::Int,    2,    2,    2,
    QMetaType::Void, 0x80000000 | 6,    2,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void RenderItem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        RenderItem *_t = static_cast<RenderItem *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->cc(); break;
        case 1: _t->addData((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< unsigned char*(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: _t->setParams((*reinterpret_cast< QDomNode(*)>(_a[1]))); break;
        case 3: _t->stop(); break;
        case 4: _t->ready(); break;
        case 5: _t->addD(); break;
        case 6: _t->sss(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (RenderItem::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&RenderItem::cc)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (RenderItem::*_t)(int , unsigned char * , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&RenderItem::addData)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (RenderItem::*_t)(QDomNode );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&RenderItem::setParams)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (RenderItem::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&RenderItem::stop)) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject RenderItem::staticMetaObject = {
    { &QQuickItem::staticMetaObject, qt_meta_stringdata_RenderItem.data,
      qt_meta_data_RenderItem,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *RenderItem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RenderItem::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_RenderItem.stringdata0))
        return static_cast<void*>(const_cast< RenderItem*>(this));
    return QQuickItem::qt_metacast(_clname);
}

int RenderItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuickItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void RenderItem::cc()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void RenderItem::addData(int _t1, unsigned char * _t2, int _t3)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void RenderItem::setParams(QDomNode _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void RenderItem::stop()
{
    QMetaObject::activate(this, &staticMetaObject, 3, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
