#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QApplication>

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
    viewer.setResizeMode(QQuickView::SizeRootObjectToView);
    viewer.setColor(QColor(0, 0, 0, 0));

    return app.exec();
}
