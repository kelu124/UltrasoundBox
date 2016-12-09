#ifndef EchoPluginsManager_H
#define EchoPluginsManager_H

#include <QObject>
#include <QHash>

#include "plugins.h"

class EchoPluginsManagerPrivate;
class EchoPluginsManager : public QObject
{
	Q_OBJECT
public:
        static EchoPluginsManager * instance() {
                static EchoPluginsManager * instance = 0;
		if (instance == 0) {
                        instance = new EchoPluginsManager;
		}
		return instance;
	}

	void init();
	void release();

    QList<VirtualScottEchoAssist *> newItems(QQuickItem *parent, QScriptEngine * engine, const int mode, int wid);
    void delItems(QList<VirtualScottEchoAssist *> items, int wid);
    void linkItems(int wid);

    QList<VirtualScottEchoAssist *> Items(int wid);

    QList <VirtualScottEchoAssist *> m_empty;
    QList <ScottEchoPlugins *>  pluginlist;
    QList <ScottEchoPlugins*>   plugin_vector[POSITION_COUNT];

private:
        EchoPluginsManager();
	void loadPlugins();
	void unloadPlugins();
	//echos hash表，由wid区分
        QHash <int, EchoPluginsManagerPrivate *> echolist;
};

#endif // EchoPluginsManager_H
