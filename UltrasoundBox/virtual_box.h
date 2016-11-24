#ifndef VIRTUALBOX_H
#define VIRTUALBOX_H

#include <QObject>
#include <QPoint>

#include <QScriptEngine>
#include <QScriptEngineDebugger>
#include <QDomDocument>
#include <QQuickItem>
#include <QQuickPaintedItem>
#include <QThread>
#include <QList>

#include <QRect>
#include <QObject>
#include <QString>

#include <QTimer>

#include "ultrasound.h"
//#include "device_ultrasound.h"


class RenderThread;
class RenderItem : public QQuickItem
{
    Q_OBJECT

public:
    RenderItem(QDomNode, QQuickItem *parent = 0);

    static QList<RenderThread *> threads;

    ~RenderItem();

    void startRenderThread();
    void stopRenderThread ();
public Q_SLOTS:
    void ready();

protected:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);

private:
    RenderThread *m_renderThread;

    QDomDocument m_config_document;
Q_SIGNALS:
    void cc();
    void addData  ( int, unsigned char *, int );
    void setParams( QDomNode);
    void stop();
public Q_SLOTS:
    void addD();
    void sss();
};

#if 0
class VirtualBox:public QObject
{
	Q_OBJECT
	//实际的图像端 图像大小.dscUpdate 之前是无效的.
    Q_PROPERTY(QRect   dscRect     READ dscRect WRITE setdscRect NOTIFY   dscRectChanged)
    Q_PROPERTY(int     dscrectx    READ dscrectx )
    Q_PROPERTY(int     dscrecty    READ dscrecty )
    Q_PROPERTY(int     dscrectw    READ dscrectw )
    Q_PROPERTY(int     dscrecth    READ dscrecth )
	/*real scan image, dscer's set*/

    Q_PROPERTY(int     id       READ id)
public:

    VirtualBox(int id
             , QDomDocument &machineXml \
             , QDomDocument &deviceXml  \
             , QDomDocument &probeXml   \
             , QDomDocument &softXml    \
             , QDomDocument &configXml  \
             , QQuickItem *gui);

	~VirtualBox();

    int     id()    const;
	/*PROPERTY*/
    int     dscrectx() const;
    int     dscrecty() const;
    int     dscrectw() const;
    int     dscrecth() const;
    QRect   dscRect()  const;
    void    setdscRect(QRect );

	int expectW() const;
	int expectH() const;
    int expectX() const;
    int expectY() const;
	void setExpectSize(int w, int h);
    void setExpectPos(int x, int  y);

	QScriptValue callerFunction(QString caller);
    QScriptValue callerFpgaFunction(QString var, QString value);
    QScriptValue callerSoftFunction(QString var, QString value);

	QScriptEngine  *engine();

private:
	void jsLoader();
	void functionLoader();
	void functionUnLoader();


    RenderItem     *m_render_ui;

	QScriptEngine  *m_engine;

    Ultrasound       *m_ultrasound;

    QDomDocument     m_machine_xml;
    QDomDocument     m_config_xml;
    QDomDocument     m_probe_xml;

    QDomDocument     m_memory_xml;

	/*Show Image*/
	QQuickItem     *m_gui_parent;

	QString         m_type;

	/*control object*/

    int	  m_expect_dsc_w, m_expect_dsc_h;
    int   m_expect_dsc_x, m_expect_dsc_y;

    int   m_dsc_image_x,  m_dsc_image_y;
    int   m_dsc_image_w,  m_dsc_image_h;

    int   m_id;

Q_SIGNALS:
    void dscRectChanged();
};
#endif
#endif // VIRTUALBOX_H
