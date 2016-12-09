#include "echo_plugins_manager.h"
#include <QQuickItem>
#include <QScriptEngine>

#include <QDomElement>
#include <QDomDocument>
#include <QDir>
#include <QPluginLoader>

#define MODULE_INFO "PluginLoader"

class EchoPluginsManagerPrivate {
public:
        EchoPluginsManagerPrivate(EchoPluginsManager *ptr):m_qptr(ptr)
	{

	}

    void newItems(QQuickItem *parent, QScriptEngine * engine, int mode)
	{
        m_qptr->pluginlist = m_qptr->plugin_vector[mode];
        foreach (ScottEchoPlugins *item,  m_qptr->pluginlist) {
            int pos, level;
            QString name, version;

            item->info(name, version, pos, level);
            qDebug() << "Info:"<<"\n\tname: "    << name
                               <<"\n\tversion: " << version
                               <<"\n\tposition: "<< pos
                               <<"\n\tlevel: "   << level;

            m_itemlist << item->newItem(parent, engine);
        }
	}

    void linkItems() {
        foreach (ScottEchoPlugins *plugin,  m_qptr->pluginlist) {
            plugin->link();
        }
    }

    void delItems(QList<VirtualScottEchoAssist *> list) {
        foreach (ScottEchoPlugins *plugin,  m_qptr->pluginlist) {
            foreach(VirtualScottEchoAssist *item, list) {
				plugin->delItem(item);
				m_itemlist.removeOne(item);
			}
		}
	}

    QList <VirtualScottEchoAssist *> m_itemlist;
        EchoPluginsManager *m_qptr;
};

EchoPluginsManager::EchoPluginsManager()
{
	setObjectName(MODULE_INFO);
}

void EchoPluginsManager::init()
{
	echolist.clear();

    for(int i = 0; i < (int)POSITION_COUNT; ++i)
        plugin_vector[i].clear();

	loadPlugins();
}

void EchoPluginsManager::release()
{
	unloadPlugins();
}

void EchoPluginsManager::loadPlugins()
{
	QDir pluginsDir("eplugins");

	foreach(QString fileName, pluginsDir.entryList(QDir::Files)) {
		QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
		QObject *plugin = loader.instance();
		if (plugin) {
			ScottEchoPlugins *item = qobject_cast <ScottEchoPlugins *> (plugin);
			if (item){
				item->init();

                int pos, level;
                QString name, version;

                item->info(name, version, pos, level);
                qDebug() << "Info:"<<"\n\tname: "    << name
                                   <<"\n\tversion: " << version
                                   <<"\n\tposition: "<< pos
                                   <<"\n\tlevel: "   << level;

                //push into plugin_vector by different pos
                int i;
                for(int j = 0; j < (int)POSITION_COUNT; ++j) {
                    if((pos >> j) & 1) {
                        for(i = 0; i < plugin_vector[j].size(); ++i) {
                            int s_pos = -1;
                            int s_level = -1;
                            QString s_name, s_version;

                            plugin_vector[j][i]->info(s_name, s_version, s_pos, s_level);
                            if(level > s_level)
                                break;
                        }
                        plugin_vector[j].insert(i, item);
                    }
                }
                qDebug() << __FUNCTION__ << fileName << "OK";
			}
		}
	}
}

void EchoPluginsManager::unloadPlugins()
{
	qDeleteAll(echolist);
	echolist.clear();
}

QList<VirtualScottEchoAssist *> EchoPluginsManager::newItems(QQuickItem *parent, QScriptEngine * engine, const int mode, int wid)
{
	if (!echolist.contains(wid)) {
                EchoPluginsManagerPrivate *dptr = new EchoPluginsManagerPrivate(this);
        dptr->newItems(parent, engine, mode);
        echolist.insert(wid, dptr);
	}
	return echolist.value(wid)->m_itemlist;
}

void EchoPluginsManager::delItems(QList<VirtualScottEchoAssist *> items, int wid)
{
	if (echolist.contains(wid)) {
		echolist.value(wid)->delItems(items);
		delete echolist[wid];
		echolist.remove(wid);
	}
}

void EchoPluginsManager::linkItems(int wid)
{
    if(echolist.contains(wid)) {
        echolist.value(wid)->linkItems();
    }
}

QList<VirtualScottEchoAssist *> EchoPluginsManager::Items(int wid)
{
	if (echolist.contains(wid))
		return echolist.value(wid)->m_itemlist;
	else
		return m_empty;
}

