TEMPLATE = app

QT += qml quick widgets script scripttools xml
CONFIG += c++11

SOURCES += main.cpp \
    virtual_box.cpp \
    device_ultrasound.cpp \
    control_object.cpp \
    ultrasound.cpp \
    echo_plugins_manager.cpp \
    render_base.cpp \
    render_b.cpp \
    render_item.cpp \
    b_params.cpp \
    c_params.cpp \
    m_params.cpp \
    d_params.cpp \
    probe_args.cpp \
    IO/mouse/mouse.cpp \
    IO/io_controls.cpp \
    layout_params.cpp \
    image_plugins_manager.cpp \
    dllmanager/dllmanager.cpp

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
    echo_plugins_manager.h \
    render_base.h \
    render_b.h \
    render_item.h \
    b_params.h \
    c_params.h \
    m_params.h \
    d_params.h \
    probe_args.h \
    IO/mouse/mouse.h \
    IO/io_controls.h \
    plugins.h \
    layout_params.h \
    image_plugins_manager.h \
    dllmanager/dllmanager.h \
    dllmanager/imageOpt.h \
    dllmanager/ZJIF_Doppler.h


DISTFILES += \
    glsl/vertex.v \
    glsl/shaderv.glsl \
    glsl/shaderf.glsl
