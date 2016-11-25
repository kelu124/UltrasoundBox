TEMPLATE = app

QT += qml quick widgets script scripttools xml
CONFIG += c++11

SOURCES += main.cpp \
    virtual_box.cpp \
    device_ultrasound.cpp \
    control_object.cpp \
    ultrasound.cpp \
    eplugins_manager.cpp \
    render_base.cpp \
    render_b.cpp \
    render_item.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    virtual_box.h \
    device_ultrasound.h \
    control_object.h \
    ultrasound.h \
    queue.h \
    eplugins_manager.h \
    render_base.h \
    render_b.h \
    render_item.h


DISTFILES += \
    glsl/vertex.v \
    glsl/shaderv.glsl \
    glsl/shaderf.glsl
