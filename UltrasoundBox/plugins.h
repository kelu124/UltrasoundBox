#ifndef PLUGINS_H
#define PLUGINS_H

#include <QQuickPaintedItem>
#include <QString>
#include <QScriptEngine>

#define POSITION_COUNT sizeof(enum EchoPosition)

enum EchoPosition {
    EchoPosition_B = 0x1,
    EchoPosition_C = 0x2,
    EchoPosition_D = 0x4,
    EchoPosition_M = 0x8
};

class VirtualScottEchoAssist {
public:
    virtual ~VirtualScottEchoAssist() {}
    virtual void link() = 0;
    virtual QQuickPaintedItem *item() = 0;
};

class ScottEchoPlugins {
public:
    virtual ~ScottEchoPlugins() {}
    virtual void init()    = 0;
    virtual void release() = 0;
    virtual void reset()   = 0;
    virtual void link()    = 0;
    virtual VirtualScottEchoAssist *newItem(QQuickItem *, QScriptEngine *) = 0;
    virtual void delItem(VirtualScottEchoAssist *) = 0;
    virtual void info(QString & name, QString & version, int &pos, int & level) = 0;
};
#define ScottEchoPlugins_iid "org.qt-project.scott.ScottEchoPlugins"
Q_DECLARE_INTERFACE(ScottEchoPlugins, ScottEchoPlugins_iid)

#endif // PLUGINS_H
