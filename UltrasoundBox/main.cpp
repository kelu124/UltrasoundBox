#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QApplication>
#include <QQuickItem>

#include <QDebug>


#include "device_ultrasound.h"
#include "virtual_box.h"

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    QQuickView  viewer;

#ifdef SCOTT_LOG
    qInstallMessageHandler(outputMessage);
#endif

    /* Translator Module */
    viewer.setSource(QUrl(QStringLiteral("qrc:/main.qml")));

    viewer.setTitle("Ultrasound Box");
    viewer.setFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowTitleHint |
        Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint/* | Qt::Popup*/);
    viewer.setMinimumSize(QSize(1024, 768));
    viewer.setColor(QColor(0, 0, 0, 0));

    QQuickItem *item = viewer.rootObject ();
    QDomDocument soft, config;

    QDomElement root = config.createElement ("RenderItem");
    config.appendChild (root);
    /*
        <render>
            <mode  value="B">
                <B x= "-1" y = "-1" w = "2" h = "2"/>
            </mode>
            <probe name="R10" icon="res/R10.png" hard="16" soft="3" type="Convex" element="128" interval="1.175"
radius="1.0" Angle="150.4"/>
            <size width=768 height=512/>

            <depth value="10.0"/>
        </render>

    */

    QDomElement render = config.createElement ("render");
    QDomElement mode   = config.createElement ("mode");
    mode.setAttribute ("value", "B");
    render.appendChild (mode);

    QDomElement B = config.createElement ("B");
    B.setAttribute ("x", "0");
    B.setAttribute ("y", "0");
    B.setAttribute ("w", "1");
    B.setAttribute ("h", "1");
    mode.appendChild (B);

    QDomElement size = config.createElement ("size");
    size.setAttribute ("width", "768");
    size.setAttribute ("height", "512");

    QDomElement probe= config.createElement ("probe");
    probe.setAttribute ("name", "R10");
    probe.setAttribute ("icon", "R10.png");
    probe.setAttribute ("hard", "16");
    probe.setAttribute ("soft", "3");
    probe.setAttribute ("type", "Convex");
    probe.setAttribute ("element", "128");
    probe.setAttribute ("radius", "6.0");
    probe.setAttribute ("angle", " 59.4");

    QDomElement scan = config.createElement ("scan");
    QDomElement depth= config.createElement ("depth");
    depth.setAttribute ("value", "9.0");
    scan.appendChild (depth);

    QDomElement complex = config.createElement ("complex");
    complex.setAttribute ("len", "1");
    QDomElement complexM= config.createElement ("M1");
    complexM.setAttribute ("angle", "0");
    complexM.setAttribute ("flag",  "0");
    QDomElement complexL= config.createElement ("L1");
    complexL.setAttribute ("angle", "-15");
    complexL.setAttribute ("flag", "1");
    QDomElement complexR= config.createElement ("R1");
    complexR.setAttribute ("angle", "15");
    complexR.setAttribute ("flag", "2");
    complex.appendChild (complexL);
    complex.appendChild (complexR);
    complex.appendChild (complexM);

    scan.appendChild (complex);

    render.appendChild (mode);
    render.appendChild (size);
    render.appendChild (probe);
    render.appendChild (scan);

    root.appendChild (render);

    Ultrasound *device = new Ultrasound(QString("device"));
    VirtualBox( 0
             , device   \
             , 0 \
             , 0 \
             , soft  \
             , config \
             , item);

    viewer.show();


    return app.exec();
}
