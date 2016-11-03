TEMPLATE = app

QT += qml quick widgets script scripttools xml
CONFIG += c++11

SOURCES += main.cpp \
    virtual_box.cpp \
    device_ultrasound.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    virtual_box.h \
    device_ultrasound.h
