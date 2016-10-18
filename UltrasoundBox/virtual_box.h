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

class RenderThread;
class VirtualBox:public QObject
{
	Q_OBJECT
	//实际的图像端 图像大小.dscUpdate 之前是无效的.
    Q_PROPERTY(QRect   dscRect     READ dscRect WRITE setdscRect NOTIFY   dscRectChanged)
    Q_PROPERTY(int     dscrectx    READ dscrectx )
    Q_PROPERTY(int     dscrecty    READ dscrecty )
    Q_PROPERTY(int     dscrectw    READ dscrectw )
    Q_PROPERTY(int     dscrecth    READ dscrecth )

	//预想的图像大小. dscUpdate之前，dscRect是无效的，所以这里用expecrtW expectH来表示预想的图像大小
	Q_PROPERTY(int expectW READ expectW)
	Q_PROPERTY(int expectH READ expectH)
    Q_PROPERTY(int expectX READ expectX)
    Q_PROPERTY(int expectY READ expectY)
	/*real scan image, dscer's set*/

	Q_PROPERTY(int     echoW    READ echoW)
	Q_PROPERTY(int     echoH    READ echoH)

public:

    VirtualBox(int id, DeviceUltrasound *device,   \
                           QDomDocument     *deviceXml, \
                           QDomDocument     *probeXml, \
                     const QDomDocument     &softXml,  \
                     const QDomDocument     &configXml, \
                           QQuickItem *gui, IOControls *io, \
                    QString runMode, QString parentMode, VirtualBoxManager *);

	VirtualBox() {}

	~VirtualBox();

    static QList <QThread *> virtual_ui_threads;

	/*PROPERTY*/
    int     dscrectx() const;
    int     dscrecty() const;
    int     dscrectw() const;
    int     dscrecth() const;
    QRect   dscRect()  const;

	int expectW() const;
	int expectH() const;
    int expectX() const;
    int expectY() const;
	void setExpectSize(int w, int h);
    void setExpectPos(int x, int  y);

	QScriptValue callerFunction(QString caller);
	QScriptValue callerFunction(QString caller,int , ...);
    QScriptValue callerFpgaFunction(QString var, QString value);
    QScriptValue callerSoftFunction(QString var, QString value);

	QScriptEngine  *engine();

public Q_SLOTS:
    void uiReady();

private:
	void jsLoader();
	void functionLoader();
	void functionUnLoader();

    RenderThread   *m_render_thread;

    QRect           m_dsc_image_rect;
	QScriptEngine  *m_engine;

	FpgaControl    *m_fpga_control;
	SoftControl    *m_soft_control;

	DeviceUltrasound *m_device_ultrasound;
	MemoryUltrasound *m_memory_ultrasound;

	Ultrasound       *m_ultrasound;

	QDomDocument    m_config_xml;
	QDomDocument    m_memory_xml;
	QDomDocument    m_soft_xml;

	QDomDocument    *m_probe_xml;
	QDomDocument    *m_device_xml;

	IOControls      *m_io;

	/*Show Image*/
	QQuickItem     *m_gui_parent;

	/*for script debugger*/
#ifdef SCRIPT_DEBUGGER
	QScriptEngineDebugger *m_debugger;
	QMainWindow *m_debugWindow;
#endif

	QString         m_type;

	/*control object*/

    /*对图像端预设的图像大小*/
	int				m_expect_dsc_w, m_expect_dsc_h;
    int             m_expect_dsc_x, m_expect_dsc_y;

	int             m_vbox_id;


	QDomDocument    m_args_layout;

	/*eplugins*/
	QList<QQuickPaintedItem *> m_eplugins;

    /**/
};

#endif // VIRTUALBOX_H
