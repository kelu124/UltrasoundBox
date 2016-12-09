#ifndef ImageProcessPlugins_H
#define ImageProcessPlugins_H
#include <QtPlugin>
#include <QScriptEngine>
#include <QQuickItem>
#include <QQuickPaintedItem>
#include "dllmanager/dllmanager.h"
enum ImageProcessPosition{
	//扫描变换前
	DscBefore = 0,
	//扫描变换后，
	DscAfter,
	//后处理前
	OptBefore,
	//后处理后
	OptAfter,
	//
	ProcessCount

};
class ImageProcessAbstract {
public:
	virtual ~ImageProcessAbstract() {}
	virtual void links() = 0;
	virtual void setMode(QString mode) = 0;
	virtual void process(unsigned char * buf, int size) = 0;
	virtual void process(QImage & image) = 0;
};
class ImageProcessPlugins {
public:
	virtual ~ImageProcessPlugins() {}
	virtual void init() = 0;
	virtual void release() = 0;
	virtual void reset() = 0;

	virtual void info (QString & name, QString & version,  ImageProcessPosition & position, int & level) = 0;

	virtual ImageProcessAbstract* newItem(QQuickItem *parent, QScriptEngine *engine, DllManagerAbstract *) = 0;
	virtual void delItem(ImageProcessAbstract *) = 0;
};
#define ImageProcessPlugins_idd "org.qt-project.ImageProcessPlugins"
Q_DECLARE_INTERFACE(ImageProcessPlugins, ImageProcessPlugins_idd)

#endif //ImageProcessPlugins_H
