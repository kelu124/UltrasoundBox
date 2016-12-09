#ifndef IMAGEPLUGINS_H
#define IMAGEPLUGINS_H

#include "image_process_plugins.h"
#include "dllmanager/dllmanager.h"

class ImageProcessItems;
class ImagePluginsManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ImagePluginsManager)
public:
    static ImagePluginsManager *instance()
    {
        static ImagePluginsManager *instance = 0;
        if (instance == 0) {
            instance = new ImagePluginsManager;
        }
        return instance;
    }

    void init();
    void release();

    void newItems(QQuickItem *parent, QScriptEngine *engine, DllManagerAbstract *, int wid);
    void delItems(int wid);

    void process(int wid, ImageProcessPosition pos, unsigned char *buf, int size);
    void process(int wid, ImageProcessPosition pos, QImage & image);
    void links(int wid);
    void setMode(int wid, QString mode);
public:
    //	QList<ImageProcessPlugins *> pluginsListArray[ImageProcessPosition::ProcessCount];
    QList<ImageProcessPlugins *> pluginsListArray[4];
private:
    ImagePluginsManager();
    void loadPlugins();
    void unloadPlugins();

    void addItem(ImageProcessPlugins *item);

    QHash<int, ImageProcessItems*> items;
};

#endif // IMAGEPLUGINS_H
