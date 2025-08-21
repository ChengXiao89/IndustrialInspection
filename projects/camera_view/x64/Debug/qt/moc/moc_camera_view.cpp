/****************************************************************************
** Meta object code from reading C++ file 'camera_view.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../camera_view.h"
#include <QtCore/qmetatype.h>
#include <QtCore/qplugin.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'camera_view.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN11camera_viewE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN11camera_viewE = QtMocHelpers::stringData(
    "camera_view"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN11camera_viewE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject camera_view::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN11camera_viewE.offsetsAndSizes,
    qt_meta_data_ZN11camera_viewE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN11camera_viewE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<camera_view, std::true_type>
    >,
    nullptr
} };

void camera_view::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<camera_view *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *camera_view::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *camera_view::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN11camera_viewE.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "interface_camera_view"))
        return static_cast< interface_camera_view*>(this);
    if (!strcmp(_clname, "com.mycompany.IPluginInterface"))
        return static_cast< IPluginInterface*>(this);
    return QObject::qt_metacast(_clname);
}

int camera_view::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    return _id;
}

#ifdef QT_MOC_EXPORT_PLUGIN_V2
static constexpr unsigned char qt_pluginMetaDataV2_camera_view[] = {
    0xbf, 
    // "IID"
    0x02,  0x78,  0x1d,  'c',  'o',  'm',  '.',  'm', 
    'y',  'c',  'o',  'm',  'p',  'a',  'n',  'y', 
    '.',  'I',  'n',  't',  'e',  'r',  'f',  'a', 
    'c',  'e',  'C',  'a',  'm',  'e',  'r',  'a', 
    // "className"
    0x03,  0x6b,  'c',  'a',  'm',  'e',  'r',  'a', 
    '_',  'v',  'i',  'e',  'w', 
    0xff, 
};
QT_MOC_EXPORT_PLUGIN_V2(camera_view, camera_view, qt_pluginMetaDataV2_camera_view)
#else
QT_PLUGIN_METADATA_SECTION
Q_CONSTINIT static constexpr unsigned char qt_pluginMetaData_camera_view[] = {
    'Q', 'T', 'M', 'E', 'T', 'A', 'D', 'A', 'T', 'A', ' ', '!',
    // metadata version, Qt version, architectural requirements
    0, QT_VERSION_MAJOR, QT_VERSION_MINOR, qPluginArchRequirements(),
    0xbf, 
    // "IID"
    0x02,  0x78,  0x1d,  'c',  'o',  'm',  '.',  'm', 
    'y',  'c',  'o',  'm',  'p',  'a',  'n',  'y', 
    '.',  'I',  'n',  't',  'e',  'r',  'f',  'a', 
    'c',  'e',  'C',  'a',  'm',  'e',  'r',  'a', 
    // "className"
    0x03,  0x6b,  'c',  'a',  'm',  'e',  'r',  'a', 
    '_',  'v',  'i',  'e',  'w', 
    0xff, 
};
QT_MOC_EXPORT_PLUGIN(camera_view, camera_view)
#endif  // QT_MOC_EXPORT_PLUGIN_V2

QT_WARNING_POP
