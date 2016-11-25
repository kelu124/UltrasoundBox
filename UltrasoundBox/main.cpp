#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QApplication>
#include <QQuickItem>

#include <QDebug>
#include <QThread>

#include "device_ultrasound.h"
#include "render_item.h"

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
//    viewer.setMinimumSize(QSize(1024, 768));
    viewer.showFullScreen ();
    viewer.setColor(QColor(255, 255, 255, 0));

    QQuickItem *item = viewer.rootObject ();
    QDomDocument soft, config;


    /*
        <render>
<RenderItem height="512" width="768">
    <render>
         <mode value=\"B\">
              <B w=\"1\" x=\"0\" y=\"0\" h=\"1\"/>
         </mode>  <size height=\"512\" width=\"768\"/>

         <probe name=\"R10\" radius=\"6.0\" icon=\"R10.png\" element=\"128\" hard=\"16\" soft=\"3\" angle=\" 59.4\" type=\"Convex\"/>
         <scan>
            <depth value=\"9.0\"/>
            <complex len=\"1\">
                <L1 angle=\"-15\" flag=\"1\"/>
                <R1 angle=\"15\" flag=\"2\"/>
                <M1 angle=\"0\" flag=\"0\"/>
            </complex>
         </scan>

        <resource>
              <b value=\"resource/greymap.dat\"/>
        </resource>
     </render>
</RenderItem>

    */

    qDebug() << item->width () << item->height () << item->objectName ();
    QDomElement root = config.createElement ("RenderItem");
    root.setAttribute ("width", "768");
    root.setAttribute ("height", "512");
    config.appendChild (root);

    QDomElement render = config.createElement ("render");
    QDomElement mode   = config.createElement ("mode");
    mode.setAttribute ("value", "B");

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

    QDomElement resource = config.createElement ("resource");
    QDomElement b = config.createElement ("b");
    b.setAttribute ("value", "resource/greymap.dat");
    resource.appendChild (b);

    render.appendChild (mode);
    render.appendChild (size);
    render.appendChild (probe);
    render.appendChild (scan);
    render.appendChild (resource);

    root.appendChild (render);

    qDebug() << config.toString ();

    RenderItem tem(root, item);


    return app.exec ();
}
