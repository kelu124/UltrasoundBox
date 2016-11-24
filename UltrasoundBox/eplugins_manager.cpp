#include "eplugins_manager.h"

#include <QDomElement>
#include <QDomDocument>
#include <QDir>
#include <QPluginLoader>

#define MODULE_INFO "PluginLoader"

class EpluginsManagerPrivate {
public:
    EpluginsManagerPrivate(EpluginsManager *ptr):m_qptr(ptr)
	{
	}
	void newItems(QQuickItem *parent, QScriptEngine * engine)
	{
		foreach (ScottEchoPlugins *item,  m_qptr->pluginlist) {
			m_itemlist << item->newItem(parent, engine);
		}
	}
	void delItems(QList<QQuickPaintedItem *> list) {
		foreach (ScottEchoPlugins *plugin,  m_qptr->pluginlist) {
			foreach(QQuickPaintedItem *item, list) {
				plugin->delItem(item);
				m_itemlist.removeOne(item);
			}
		}
	}

	QList <QQuickPaintedItem *> m_itemlist;
    EpluginsManager *m_qptr;
};

EpluginsManager::EpluginsManager()
{
	setObjectName(MODULE_INFO);
}

void EpluginsManager::init()
{
	echolist.clear();
	pluginlist.clear();
	loadPlugins();
}

void EpluginsManager::release()
{
	unloadPlugins();
}

void EpluginsManager::loadPlugins()
{
    QDir pluginsDir(qApp->applicationDirPath() + "/third/eplugins");
	foreach(QString fileName, pluginsDir.entryList(QDir::Files)) {
		QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
		QObject *plugin = loader.instance();
		if (plugin) {
			ScottEchoPlugins *item = qobject_cast <ScottEchoPlugins *> (plugin);
			if (item){
				item->init();
				pluginlist << item;
                qDebug() << __FUNCTION__ << fileName << "OK";
			}
		}
		else {
			//			qDebug() << "Warnning: " << __FILE__ << __LINE__ << loader.errorString();
		}
	}
}

void EpluginsManager::unloadPlugins()
{
	qDeleteAll(echolist);
	echolist.clear();
}

QList<QQuickPaintedItem *> EpluginsManager::newItems(QQuickItem *parent, QScriptEngine * engine, int wid)
{
	if (!echolist.contains(wid)) {
        EpluginsManagerPrivate *dptr = new EpluginsManagerPrivate(this);
		dptr->newItems(parent, engine);
		echolist.insert(wid, dptr);
	}
	return echolist.value(wid)->m_itemlist;
}

void EpluginsManager::delItems(QList<QQuickPaintedItem *> items, int wid)
{
	if (echolist.contains(wid)) {
		echolist.value(wid)->delItems(items);
		delete echolist[wid];
		echolist.remove(wid);
	}
}
QList <QQuickPaintedItem *> EpluginsManager::Items(int wid)
{
	if (echolist.contains(wid))
		return echolist.value(wid)->m_itemlist;
	else
		return m_empty;
}

