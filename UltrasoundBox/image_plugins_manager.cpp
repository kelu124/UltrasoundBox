#include "image_plugins_manager.h"

#include <QDir>
#include <QPluginLoader>

#define MODULE_INFO "ImagePluginLoader"

class ImageProcessItems {
public:
    ImageProcessItems(ImagePluginsManager *ptr):m_q(ptr)
    {}
    virtual ~ImageProcessItems()
    {
        delItems();
    }

    void newItems(QQuickItem *parent, QScriptEngine *engine, DllManagerAbstract *dllabs)
    {
        for (int i = 0; i < sizeof(itemsArray)/ sizeof(itemsArray[0]); i++) {
            foreach (ImageProcessPlugins *item, m_q->pluginsListArray[i]) {
                itemsArray[i].append((item->newItem(parent, engine, dllabs)));
            }
        }
    }

    void delItems()
    {
        for (int i = 0; i < sizeof(itemsArray)/ sizeof(itemsArray[0]); i++) {
            foreach (ImageProcessPlugins *item, m_q->pluginsListArray[i]) {
                foreach (ImageProcessAbstract *it, itemsArray[i]) {
                    item->delItem(it);
                }
                itemsArray[i].clear();
            }
        }
    }

    void process(ImageProcessPosition position, unsigned char *buf, int size)
    {
        if (0 <= position && position< ImageProcessPosition::ProcessCount) {
            foreach(ImageProcessAbstract *abs, itemsArray[position]) {
                abs->process(buf, size);
            }
        }
    }
    void process(ImageProcessPosition position, QImage &image)
    {
        if (0 <= position && position< ImageProcessPosition::ProcessCount) {
            foreach(ImageProcessAbstract *abs, itemsArray[position]) {
                abs->process(image);
            }
        }
    }
    void setMode( QString mode)
    {
        for (int i = 0; i < sizeof(itemsArray)/ sizeof(itemsArray[0]); i++)  {
            foreach(ImageProcessAbstract *abs, itemsArray[i]) {
                abs->setMode(mode);
            }
        }
    }
    void links()
    {
        for (int i = 0; i < sizeof(itemsArray)/ sizeof(itemsArray[0]); i++) {
            foreach(ImageProcessAbstract * abs, itemsArray[i]) {
                abs->links();
            }
        }
    }
public:
    QList<ImageProcessAbstract*> itemsArray[ImageProcessPosition::ProcessCount];
    ImagePluginsManager *m_q;
};


ImagePluginsManager::ImagePluginsManager()
{
    setObjectName(MODULE_INFO);
}

void ImagePluginsManager::init()
{
    for(int i = 0 ; i < sizeof(pluginsListArray)/sizeof(pluginsListArray[0]); i++ ) {
        pluginsListArray[i].clear();
    }
    items.clear();
    loadPlugins();
}
void ImagePluginsManager::release()
{
    qDeleteAll(items);
    items.clear();
    unloadPlugins();
}
void ImagePluginsManager::loadPlugins()
{
    QDir pluginsDir("imageplugin");
    foreach (QString filename, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(filename));
        QObject *plugin = loader.instance();
        qDebug()<<__FUNCTION__ <<filename;
        if (plugin) {
            ImageProcessPlugins *item = qobject_cast<ImageProcessPlugins *>(plugin);
            if(item) {
                qDebug()<<__FUNCTION__<<"OK";
                item->init();
                addItem(item);
            }
        }
    }
}
void ImagePluginsManager::addItem(ImageProcessPlugins *item)
{
    QHash<int, QString> pos;
    pos[DscBefore] = "DscBefore";
    pos[DscAfter] = "DscAfter";
    pos[OptBefore] = "OptBefore";
    pos[OptAfter] = "OptAfter";


    QString name = "";
    QString version = "";
    int level = -1;
    ImageProcessPosition position = ImageProcessPosition::ProcessCount;

    item->info(name, version, position, level);
    qDebug()<<__FUNCTION__ << "plugin info:"
           <<"\n\tname: "<< name
          <<"\n\tversion: "<< version
         <<"\n\tposition: "<< pos[position]
           <<"\n\tlevel: "<< level;

    //level从大到小的方式存储
    int i;
    for (i = 0; i < pluginsListArray[position].size(); i++) {

        QString tname = "";
        QString tversion = "";
        int tlevel = -1;
        ImageProcessPosition tposition = ImageProcessPosition::ProcessCount;

        pluginsListArray[position][i]->info(tname, tversion, tposition, tlevel);
        if (level > tlevel) {
            //终于找到一个比我矮的了，你的位置是i, 我要插队
            break;
        }
    }
    pluginsListArray[position].insert(i, item);
}
void ImagePluginsManager::unloadPlugins()
{
    for(int i = 0 ; i < sizeof(pluginsListArray)/sizeof(pluginsListArray[0]); i++ ) {
        foreach (ImageProcessPlugins *item, pluginsListArray[i]) {
            item->release();
        }
        pluginsListArray[i].clear();
    }
}
void ImagePluginsManager::newItems(QQuickItem *parent, QScriptEngine * engine, DllManagerAbstract *dllabs, int wid)
{
    if (!items.contains(wid)) {
        ImageProcessItems *item = new ImageProcessItems(this);
        item->newItems(parent, engine, dllabs);
        items.insert(wid, item);
    }
}


void ImagePluginsManager::delItems(int wid)
{
    if (items.contains(wid)) {
        items.value(wid)->delItems();
        ImageProcessItems *item = items[wid];
        delete item;
        item = NULL;
        //		delete items[wid];
        items.remove(wid);
    }
}

void ImagePluginsManager::process(int wid, ImageProcessPosition pos, unsigned char *buf, int size)
{
    if (items.contains(wid)) {
        items.value(wid)->process(pos, buf, size);
    }
}
void ImagePluginsManager::process(int wid, ImageProcessPosition pos, QImage & image)
{
    if (items.contains(wid)) {
        items.value(wid)->process(pos, image);
    }
}
void ImagePluginsManager::links(int wid)
{
    if(items.contains(wid)) {
        items.value(wid)->links();
    }
}
void ImagePluginsManager::setMode(int wid, QString mode)
{
    if (items.contains(wid)) {
        items.value(wid)->setMode(mode);
    }
}
