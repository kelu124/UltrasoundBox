#ifndef EPLUGINSMANAGER_H
#define EPLUGINSMANAGER_H


#include <QObject>
#include <QHash>
#include <QQuickItem>
#include <QQuickPaintedItem>
#include <QScriptEngine>

class ScottEchoPlugins {
public:
    virtual ~ScottEchoPlugins() {}
    virtual void init()    = 0;
    virtual void release() = 0;
    virtual void reset()   = 0;
    virtual QQuickPaintedItem *newItem(QQuickItem *, QScriptEngine *) = 0;
    virtual void delItem(QQuickPaintedItem *) = 0;
};
#define ScottEchoPlugins_iid "org.qt-project.scott.ScottEchoPlugins"
Q_DECLARE_INTERFACE(ScottEchoPlugins, ScottEchoPlugins_iid)

class EpluginsManagerPrivate;
class EpluginsManager : public QObject
{
	Q_OBJECT
public:
    static EpluginsManager * instance() {
        static EpluginsManager * instance = 0;
		if (instance == 0) {
            instance = new EpluginsManager;
		}
		return instance;
	}

	void init();
	void release();

	QList<QQuickPaintedItem *> newItems(QQuickItem *parent, QScriptEngine * engine, int wid);

	void delItems(QList<QQuickPaintedItem *> items, int wid);

	QList<QQuickPaintedItem *> Items(int wid);

	QList <QQuickPaintedItem *> m_empty;
    QList <ScottEchoPlugins *>  pluginlist;
private:
    EpluginsManager();
	void loadPlugins();
	void unloadPlugins();
	//echos hash表，由wid区分
	QHash <int, EpluginsManagerPrivate *> echolist;
};

#endif // PLUGINSMANAGER_H
