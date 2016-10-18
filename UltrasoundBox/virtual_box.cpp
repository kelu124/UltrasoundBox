#include "virtual_box.h"

#include <QDebug>

#include <QMutex>
#include <QThread>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <qsgsimpletexturenode.h>

#include <qopenglshaderprogram.h>
#include <qopenglfunctions.h>

#include <QQuickWindow>

QList <QThread *> VirtualBox::virtual_ui_threads;

class Render :protected QOpenGLFunctions
{
};

class RenderThread : public QThread
{
    Q_OBJECT
public:
    RenderThread(const QSize &size)
        : surface(0)
        , context(0)
        , m_renderFbo(0)
        , m_displayFbo(0)
//        , m_logoRenderer(0)
        , m_size(size)
    {
        VirtualBox::virtual_ui_threads << this;
    }

    QOffscreenSurface *surface;
    QOpenGLContext    *context;

public slots:
    void renderNext()
    {
        context->makeCurrent(surface);

        if (!m_renderFbo) {
            // Initialize the buffers and renderer
            QOpenGLFramebufferObjectFormat format;
            format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
            m_renderFbo  = new QOpenGLFramebufferObject (m_size, format);
            m_displayFbo = new QOpenGLFramebufferObject(m_size, format);
//            m_logoRenderer = new LogoRenderer();
//            m_logoRenderer->initialize();
        }

        m_renderFbo->bind();
        context->functions ()->glViewport (0, 0, m_size.width(), m_size.height());

//        m_logoRenderer->render();

        // We need to flush the contents to the FBO before posting
        // the texture to the other thread, otherwise, we might
        // get unexpected results.
        context->functions()->glFlush();

        m_renderFbo->bindDefault();
        qSwap(m_renderFbo, m_displayFbo);

        emit textureReady(m_displayFbo->texture(), m_size);
    }

    void shutDown()
    {
        context->makeCurrent(surface);
        delete m_renderFbo;
        delete m_displayFbo;
//        delete m_logoRenderer;
        context->doneCurrent();
        delete context;

        // schedule this to be deleted only after we're done cleaning up
        surface->deleteLater();

        // Stop event processing, move the thread to GUI and make sure it is deleted.
        exit();
        moveToThread(QGuiApplication::instance()->thread());
    }

signals:
    void textureReady(int id, const QSize &size);

private:
    QOpenGLFramebufferObject *m_renderFbo;
    QOpenGLFramebufferObject *m_displayFbo;

    //LogoRenderer *m_logoRenderer;
    QSize m_size;
};


class TextureNode : public QObject, public QSGSimpleTextureNode
{
    Q_OBJECT

public:
    TextureNode(QQuickWindow *window)
        : m_id(0)
        , m_size(0, 0)
        , m_texture(0)
        , m_window(window)
    {
        // Our texture node must have a texture, so use the default 0 texture.
        m_texture = m_window->createTextureFromId(0, QSize(1, 1));
        setTexture(m_texture);
        setFiltering(QSGTexture::Linear);
    }

    ~TextureNode()
    {
        delete m_texture;
    }

signals:
    void textureInUse();
    void pendingNewTexture();

public slots:

    // This function gets called on the FBO rendering thread and will store the
    // texture id and size and schedule an update on the window.
    void newTexture(int id, const QSize &size) {
        m_mutex.lock();
        m_id = id;
        m_size = size;
        m_mutex.unlock();

        // We cannot call QQuickWindow::update directly here, as this is only allowed
        // from the rendering thread or GUI thread.
        emit pendingNewTexture();
    }


    // Before the scene graph starts to render, we update to the pending texture
    void prepareNode() {
        m_mutex.lock();
        int newId = m_id;
        QSize size = m_size;
        m_id = 0;
        m_mutex.unlock();
        if (newId) {
            delete m_texture;
            // note: include QQuickWindow::TextureHasAlphaChannel if the rendered content
            // has alpha.
            m_texture = m_window->createTextureFromId(newId, size);
            setTexture(m_texture);

            markDirty(DirtyMaterial);

            // This will notify the rendering thread that the texture is now being rendered
            // and it can start rendering to the other one.
            emit textureInUse();
        }
    }

private:

    int m_id;
    QSize m_size;

    QMutex m_mutex;

    QSGTexture *m_texture;
    QQuickWindow *m_window;
};



#define CHECK_DIVISOR(var) do { \
	if ((var) == 0) { \
	qDebug() << __FILE__<<__LINE__<< "if you want to div 0, it's a thrilling thing."; \
	} \
	}while(0);
VirtualBox::VirtualBox(int id, DeviceUltrasound *device,   \
					   QDomDocument     *deviceXml, \
					   QDomDocument     *probeXml, \
					   const QDomDocument     &softXml,  \
					   const QDomDocument     &configXml, \
					   const QByteArray       &js,       \
					   QQuickItem *gui, IOControls *io, qreal dscW, qreal dscH, QString runMode, QString parentMode, VirtualBoxManager *manager)
{
#ifdef SCRIPT_DEBUGGER
	m_debugger = nullptr;
	m_debugWindow = nullptr;
#endif

	Q_ASSERT(io);
	m_vbox_id= id;

	Q_UNUSED(dscW);
	Q_UNUSED(dscH);

	m_dsc_image_rect = QRect (0, 0, dscW, dscH);

	m_replay_w = 0;
	m_replay_h = 20;

	m_parent_mode = parentMode;
	m_mode = runMode;
	m_manager = manager;
	/*member variables assignment*/
	m_device_xml = deviceXml;
	m_probe_xml  = probeXml;
	m_soft_xml   = softXml.cloneNode().toDocument();
	m_config_xml = configXml.cloneNode().toDocument();
	m_js_buffer  = js;
	m_gui_parent = gui;


	/*engine init*/
	m_engine = new QScriptEngine;

	/*engine object init*/
	m_fpga_control = new FpgaControl(m_device_xml);
	m_fpga_control->engineLinked(m_engine);

	m_soft_control = new SoftControl(&m_soft_xml);
	m_soft_control->engineLinked(m_engine);

	m_echo = new DeviceEcho(m_gui_parent);
	m_echo->setVisible(true);

	m_device_ultrasound = device;
	m_memory_ultrasound = 0;
	m_ultrasound        = 0;

	m_type              = QString();
	m_io                = io;

	m_probe_info = new ProbeInfo(m_probe_xml, m_engine);

	/*Register VirtualBox into engine*/
	m_engine->globalObject().setProperty("vbox", m_engine->newQObject(this));

	jsLoader();
	functionLoader();

	/*load eplugins*/
	m_eplugins = PluginsManager::instance()->newItems(m_echo, m_engine, m_vbox_id);

	m_args_layout.clear();

	m_freeze = false;

}

VirtualBox::~VirtualBox()
{
	if (m_memory_ultrasound) {
		delete m_memory_ultrasound;
		m_memory_ultrasound = 0;
	}

	if (m_type == "device") {
		//vbox析构时，从Cache中删除
		//		m_device_ultrasound->delCacheUser(m_vbox_id);
		m_device_ultrasound->decRef();
	}
	functionUnLoader();

	PluginsManager::instance()->delItems(m_eplugins, m_vbox_id);

	delete m_probe_info;
	delete m_echo;

	delete m_soft_control;
	delete m_fpga_control;
	delete m_engine;
#ifdef SCRIPT_DEBUGGER
	if (m_debugger) {
		m_debugger->detach();
		delete m_debugger;
		m_debugger = nullptr;
	}
	//NOTE Don't try to delete  m_debugWindow, it is created by  QScriptEngineDebugger and will destory by QScriptEngineDebugger;
	//	if (m_debugWindow) {
	//		delete m_debugWindow;
	//		m_debugWindow = nullptr;
	//	}
#endif
}

int VirtualBox::echoH() const
{
	return m_echo->height();
}

int VirtualBox::echoW() const
{
	return m_echo->width();
}

int VirtualBox::ID()
{
	return m_vbox_id;
}

QScriptValue VirtualBox::callerFunction(QString function, int len, ...)
{
	va_list arg_ptr;
	QStringList args;
	QScriptValue ret;
	va_start(arg_ptr,len);
	do
	{
		args << va_arg(arg_ptr,QString);

	} while(--len);
	va_end(arg_ptr);

	QScriptValue caller = m_engine->globalObject().property(function);
	if (!caller.isValid())
		return caller;
	else {
		QScriptValueList list;
		for (int i = 0; i < args.length(); i++) {
			QScriptValue v(args.at(i));
			list << v;
		}
		ret = caller.call(QScriptValue(), list);
	}
	return ret;
}

QScriptValue VirtualBox::callerFunction(QString function)
{
	QScriptValue ret;
	QScriptValue caller = m_engine->globalObject().property(function);

	if (!caller.isValid()) {
		return caller;
	}else {
		ret = caller.call(QScriptValue());
	}
	return ret;
}

/*TCP/IP*/
QScriptValue VirtualBox::callerFpgaFunction(QString var, QString value)
{
	m_fpga_control->setObjectValueFName(var, value);
	m_ultrasound->updateCtrlTable(true);

	return QScriptValue();
}

QScriptValue VirtualBox::callerSoftFunction(QString var, QString value)
{
	m_soft_control->setObjectValueFName(var, value);
	return QScriptValue();
}

void VirtualBox::callBrothersfunction(QString type, QString function, QString brotherMode)
{
	Q_UNUSED(type);
	QScriptValue ret = m_manager->callerFunctionByMode(function, brotherMode);
	if (ret.isError()) {
		qDebug()<<__FILE__<<__LINE__<<"call Function Error"<<function <<brotherMode;
	}
}
void VirtualBox::realUpFpga()
{
	m_ultrasound->updateCtrlTable(false);
}

void VirtualBox::functionUnLoader()
{
	delete m_replayer;
	delete m_mouse;
	delete m_dsc;
	delete m_focus;
	delete m_roi;
	delete m_pwParams;
	delete m_mParams;
	delete m_pwSampling;
	delete m_pwCaliper;
	delete m_vcaliper;
	delete m_hcaliper;
	delete m_colormap;
	delete m_mCaliper;
	delete m_mSamplingLine;
	delete m_MDSampling;
	delete m_mTgc;
	delete m_selectedMark;
	delete m_args_params;
	delete m_magnify;
}

void VirtualBox::functionLoader()
{
    /*add instance function */
    /*bar*/
    m_status_bar    = StatusBar::instance();
    m_engine->globalObject ().setProperty ("StatusBar", m_engine->newQObject (m_status_bar));

    if (m_engine->hasUncaughtException ()){
        qWarning() << "Error:" << __FILE__ << __LINE__ << "StatusBar " << m_engine->uncaughtExceptionLineNumber () << " " << m_engine->uncaughtException ().toString ();
    }
    /*bar*/

    /*add Info*/
    m_info          = Info::instance();
    m_engine->globalObject().setProperty("Info", m_engine->newQObject(m_info));

    if (m_engine->hasUncaughtException()) {
        qWarning() << "Error: " << __FILE__ << __LINE__ << "Info " << m_engine->uncaughtException().toString();
    }
    /*Info*/
	/*add function! */
	QDomNode node = m_config_xml.documentElement ();
	/*io*/
	m_mouse      = new MouseWidget(m_engine, node);

	//NOTE functionLoader
	m_colormap      = new ColorMap(m_engine, node);
	m_mTgc          = new TgcParams(m_engine, node);
	m_selectedMark  = new SelectedMark(m_engine, node);
	m_hcaliper      = new HCaliper(m_engine, node);
	m_vcaliper      = new VCaliper(m_engine, node);
	m_pwCaliper     = new PwCaliper(m_engine, node);
	m_pwSampling    = new PwSampling(m_engine,  node);
	m_pwParams      = new PWParams(m_engine, node);
	m_mParams		= new MParams(m_engine, node);
	m_roi           = new Roi(m_engine, node);
	m_focus         = new Focus(m_engine, node);
	m_mCaliper      = new MCaliper(m_engine, node);
	m_mSamplingLine = new MSamplingLine(m_engine, node);
	m_MDSampling	= new MDSampling(m_engine, node);
	m_args_params   = new ArgsParams(m_engine, node);
	m_magnify		= new Magnify(m_engine);


	Args::instance()->addEngine(m_vbox_id, m_args_params);

	/*dsc*/
	m_dsc           = new DSC(m_engine, node);
	//Add by jiawentao 2016/7/22, dscupdate后，通过dbus 获取实际的dsc大小，给Roi等插件使用
	connect(m_dsc, &DSC::dscUpdateSignal, [=](){ setdscRect(VDBus::instance()->dscInterface()->getImgRect(m_vbox_id)); });


	m_replayer   = new Replayer(m_engine, node);
	ReplayManager::instance()->regisiterObj(m_vbox_id, m_replayer);

	connect(m_mouse, SIGNAL(move(QPoint)), m_replayer, SIGNAL(move(QPoint)));
	connect(m_mouse, SIGNAL(leftClicked()), m_replayer, SIGNAL(leftClicked()));
	connect(m_mouse, SIGNAL(rightClicked()), m_replayer, SIGNAL(rightClicked()));

	//ReplayManager::instance()->setMouseControl(m_mouse);
	connect(m_dsc,           SIGNAL(colorMap(QImage &,QImage &, QImage &)), m_colormap, SIGNAL(colorMapChanged(QImage &,QImage &, QImage &)));
	connect(m_roi,           SIGNAL(fpgaChanged()),                         this,       SLOT(fpgaControlUpdate()));
	connect(m_mSamplingLine, SIGNAL(fpgaChanged()),                         this, SLOT(fpgaControlUpdate()));
	connect(m_pwSampling,    SIGNAL(fpgaChanged()),                         this, SLOT(fpgaControlUpdate()));
}
//ADD byjiawentao 2016/8/2, brief:这个invokable 开放给脚本，切换模式的时候，DSCUpdate之后就调用一下，能保证Roi与真实图像一致
void VirtualBox::getDscRect()
{
//	qDebug()<<__FUNCTION__<< "before "<<m_type << m_mode<<m_vbox_id << dscRect();
	setdscRect(VDBus::instance()->dscInterface()->getImgRect(m_vbox_id));
//	qDebug()<<__FUNCTION__<< "after "<<m_type << m_mode<<m_vbox_id << dscRect();
}

void VirtualBox::jsLoader()
{
	QString contentJS(m_js_buffer);

#ifdef SCRIPT_DEBUGGER
	if (!m_debugger) {
		m_debugger = new QScriptEngineDebugger(this);
		m_debugWindow = m_debugger->standardWindow();
		//		m_debugWindow->setWindowModality(Qt::ApplicationModal);
		m_debugWindow->resize(1024, 768);

	}
	m_debugger->attachTo(m_engine);
	m_debugger->action(QScriptEngineDebugger::InterruptAction)->trigger();
#endif

	//NOTE Just check js syntax, however there is no use. it cann't check running exception. Everywhere you call QScriptValue::call(), the call result also can throw a exception.

	//	QScriptSyntaxCheckResult result = m_engine->checkSyntax(contentJS);
	//	if (result == QScriptSyntaxCheckResult::Error) {
	//		qDebug()<<"JS check...\n\r Error:" << result.errorLineNumber()<< result.errorColumnNumber() <<result.errorLineNumber();
	//	} else if (result == QScriptSyntaxCheckResult::Intermediate) {
	//		qDebug()<<"JS check...\n\r Intermediate: The program is incomplete."<<result.errorMessage();
	//	} else {
	//		qDebug()<<"JS check...\n\r OK";
	//	}

	m_engine->evaluate (contentJS);

	if(m_engine->hasUncaughtException()) {
		qDebug() << "Error:" << __FILE__ << __LINE__<< "jsLoader: evaluate() "<< m_engine->uncaughtExceptionLineNumber() \
				 << " "<< m_engine->uncaughtException ().toString ();
		m_engine->clearExceptions();
	}
	//NOTE by jiawentao 2016/8/3, detail:定时器轮询，捕获JS运行时异常。如果影响效率，可以去掉。去掉的代价是，脚本开放后，运行时异常不能观察到。
	//暂时没有更好的方法。


    /* add third js folder*/
    QDir pluginsDir(QDir::currentPath() + "/third/js");

    foreach(QString fileName, pluginsDir.entryList(QDir::Files)) {
        QFile file(QDir::currentPath() + "/third/js" + fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        QByteArray content =  file.readAll();

        m_engine->evaluate (content);

        if(m_engine->hasUncaughtException()) {
            qDebug() << "Error:" << "third/js/" + fileName << "jsLoader: evaluate() "<< m_engine->uncaughtExceptionLineNumber() \
                     << " "<< m_engine->uncaughtException ().toString ();
            m_engine->clearExceptions();
        }
    }

	startTimer(100);

}
void VirtualBox::timerEvent(QTimerEvent *)
{
	if (m_engine->hasUncaughtException()) {
		qDebug()<<__FILE__ << __LINE__ << "JS throw  Exception: at line " << m_engine->uncaughtExceptionLineNumber() \
			   <<" " << m_engine->uncaughtException ().toString ();
		m_engine->clearExceptions();
	}
}
void VirtualBox::PowerOn()
{
}

void VirtualBox::PowerOff()
{

}

void VirtualBox::setEchoRect(qreal x, qreal y, qreal w, qreal h)
{
	qDebug()<<__FILE__<<__FUNCTION__<<x << y << w << h;
	m_echo->setEchoRect(x, y, w, h);
}
void VirtualBox::goVirtual()
{
	if (m_memory_ultrasound ) {
		delete m_memory_ultrasound;
		m_memory_ultrasound = 0;
	}
	m_type = "memory";
	/*备份m_device到MemoryUltrasound*/
	m_memory_xml = m_device_xml->cloneNode().toDocument();
	m_memory_ultrasound = new MemoryUltrasound(&m_memory_xml);

	m_fpga_control->changedSource(&m_memory_xml);
	m_ultrasound = m_memory_ultrasound;

	m_device_ultrasound->decRef();

	VDBus::instance()->imageInterface()->setFreeze(m_vbox_id);
	int maxFrame = VDBus::instance()->dscInterface()->getMaxReplay(m_vbox_id);
	QString fps = VDBus::instance()->dscInterface()->getFps(m_vbox_id);

	qDebug() << __FILE__<<__LINE__<<"get maxFrame:"<<maxFrame <<"fps:"<< fps;

	CHECK_DIVISOR(fps.toFloat());
	m_replayer->setSpeed(1000/fps.toFloat());
	m_replayer->setMin(0);
	m_replayer->setMax(maxFrame);
	m_replayer->setStart(0);
	m_replayer->setEnd(maxFrame);
	m_replayer->setCurr(maxFrame);
	m_replayer->upParams();
    m_replayer->setIsAuto(true);

	connect (ReplayManager::instance(), SIGNAL(currentFrame(int, int)), this, SLOT(replayerFrame(int, int)));
}

void VirtualBox::goReal()
{

	m_ultrasound = m_device_ultrasound;

	if (m_memory_ultrasound) {
		delete m_memory_ultrasound;
		m_memory_ultrasound = 0;
	}
	m_fpga_control->changedSource(m_device_xml);

	m_replayer->setIsAuto(false);
	m_replayer->upParams();

	if (m_type == "memory") {
		VDBus::instance()->imageInterface()->setRunning(m_vbox_id);
		disconnect (ReplayManager::instance(), SIGNAL(currentFrame(int, int)), this, SLOT(replayerFrame(int, int)));
	}

	m_type = "device";
	m_device_ultrasound->addRef();
}

/*Q_INVOKABLE*/
void VirtualBox::switchUltrasound(QString source)
{
	if (source == m_type) return;

	if (source == "device") {
		goReal();
	} else if (source == "memory") {
		goVirtual();
	} else if (source == "files") {
		//        goFile();
	} else {
		goReal();
	}
}

void VirtualBox::switchFocus(bool flag)
{
	if (true == flag) {
		connect(m_io, SIGNAL(move(QPoint)),   m_mouse, SIGNAL(move(QPoint))    );
		connect(m_io, SIGNAL(leftClicked()),  m_mouse, SIGNAL(leftClicked())   );
		connect(m_io, SIGNAL(rightClicked()), m_mouse, SIGNAL(rightClicked())  );
		connect(m_io, SIGNAL(stateChanged()), m_mouse, SIGNAL(paramsChanged()) );
	}else {
		disconnect(m_io, SIGNAL(move(QPoint)),   m_mouse, SIGNAL(move(QPoint))    );
		disconnect(m_io, SIGNAL(leftClicked()),  m_mouse, SIGNAL(leftClicked())   );
		disconnect(m_io, SIGNAL(rightClicked()), m_mouse, SIGNAL(rightClicked())  );
		disconnect(m_io, SIGNAL(stateChanged()), m_mouse, SIGNAL(paramsChanged()) );
	}
}


QString VirtualBox::mode() const
{
	return m_mode;
}

void VirtualBox::setMode(QString mode)
{
	m_mode = mode;
	QScriptValue ret;
	if (mode == "B") {
		m_ultrasound->modeB();
		QScriptValue mode = m_engine->globalObject().property("ModeB");
		ret = mode.call(QScriptValue());

	}else if (mode == "C"){
		m_ultrasound->modeC();
		QScriptValue mode = m_engine->globalObject().property("ModeC");
		ret = mode.call(QScriptValue());
	}else if (mode == "M") {
		m_ultrasound->modeM();
		QScriptValue mode = m_engine->globalObject().property("ModeM");
		ret = mode.call(QScriptValue());
	}else if (mode == "D") {
		m_ultrasound->modeD();
		QScriptValue mode = m_engine->globalObject().property("ModeD");
		ret = mode.call(QScriptValue());
	}else {
		qDebug()<<"no such mode"<< mode;
		return;
	}
	if (!ret.isValid()) {
		qDebug()<<"setMode call JS failed, mode:"<< mode;
	}
	VDBus::instance()->imageInterface()->setRunning(m_vbox_id);

	realUpFpga();
	emit modeChanged();
}

bool VirtualBox::selected() const
{
	return m_selectedMark->selected();
}

void VirtualBox::setSelected(bool b)
{
	//qDebug () << __FILE__<< __LINE__ <<b << ID();
	emit m_replayer->selected(b);
	if (b == selected()) return ;
	m_selectedMark->setselected(b);
	switchFocus(b);
	emit selectedChanged();
}

void VirtualBox::fpgaControlUpdate()
{
	if (m_type == "device") {
		Args::instance()->updateArgs();
		m_device_ultrasound->updateCtrlTable(false);
	}
}

QScriptEngine *VirtualBox::engine()
{
	return m_engine;
}

void VirtualBox::setFreeze(bool v)
{
	if (v == m_freeze) return;

	m_freeze = v;
	QString type = m_freeze ? "memory" : "device";

	switchUltrasound( type );
	emit freezeChanged();
}

bool VirtualBox::freeze() const
{
	return m_freeze;
}

QString VirtualBox::getType() const
{
	return m_type;
}

QString VirtualBox::managerMode() const
{
	return m_parent_mode;
}

void VirtualBox::log(QVariant log)
{
#ifdef SCRIPT_LOG_SUPPORT
	qDebug() << __FILE__<<__FUNCTION__<< log;
#endif
}
void VirtualBox::stringLog(QString log)
{
	qDebug() << __FILE__ << __FUNCTION__ << log;
}

void VirtualBox::numLog(qreal log)
{
	qDebug() << __FILE__ << __FUNCTION__ <<log;
}

Replayer* VirtualBox::replayerInstance()
{
	return m_replayer;
}

void VirtualBox::layout(QString parent, QString me, int id, QString mode, QRect &rect)
{
	m_layout_w = parent + ":" + me;

	VDBus::instance()->imageInterface()->SetUIParams(parent, me, id, mode, rect);
}

QRect VirtualBox::dscRect() const
{
	return m_dsc_image_rect;
}

void VirtualBox::setdscRect(QRect r)
{
//	qDebug()<<__FUNCTION__<<r;
	if (r == m_dsc_image_rect) {
		return;
	}

	m_dsc_image_rect = r;

	emit dscRectChanged();
}

int VirtualBox::dscrectx() const
{
	return m_dsc_image_rect.x();
}
int VirtualBox::dscrecty() const
{
	return m_dsc_image_rect.y();
}
int VirtualBox::dscrectw() const
{
	return m_dsc_image_rect.width();
}
int VirtualBox::dscrecth() const
{
	return m_dsc_image_rect.height();
}
void VirtualBox::replayerFrame(int n, int id)
{
	//id match should replay, or id equal -1 means allReplay
	if (m_vbox_id == id || -1 == id) {
		VDBus::instance()->imageInterface()->ReplayFrame(m_vbox_id, n);
	}
}

int VirtualBox::expectW() const
{
	return m_expect_dsc_w;
}
int VirtualBox::expectH() const
{
	return m_expect_dsc_h;
}
int VirtualBox::expectX() const
{
    return m_expect_dsc_x;
}
int VirtualBox::expectY() const
{
    return m_expect_dsc_y;
}

void VirtualBox::setExpectSize(int w, int h)
{
	m_expect_dsc_w = w;
	m_expect_dsc_h = h;
}

void VirtualBox::setExpectPos(int x, int y)
{
    m_expect_dsc_x = x;
    m_expect_dsc_y = y;
}


void VirtualBox::turnLR(int v)
{
    QScriptValue turn = m_engine->globalObject().property("turnLR");
    if (turn.isFunction()) {
        QScriptValueList args;
        args<< v;
        turn.call(QScriptValue(), args);
    }
}
void VirtualBox::turnUD(int v)
{
    QScriptValue turn = m_engine->globalObject().property("turnUD");
    if (turn.isFunction()) {
        QScriptValueList args;
        args<< v;
        turn.call(QScriptValue(), args);
    }
}
void VirtualBox::rotate(int angle)
{
    QScriptValue turn = m_engine->globalObject().property("rotate");
    if (turn.isFunction()) {
        QScriptValueList args;
        args<< angle;
        turn.call(QScriptValue(), args);
    }
}
void VirtualBox::inverseGary(int v)
{
    QScriptValue turn = m_engine->globalObject().property("inverseGary");
    if (turn.isFunction()) {
        QScriptValueList args;
        args<< v;
        turn.call(QScriptValue(), args);
    }
}
void VirtualBox::inverseBlood(int v)
{
    QScriptValue turn = m_engine->globalObject().property("inverseBlood");
    if (turn.isFunction()) {
        QScriptValueList args;
        args<< v;
        turn.call(QScriptValue(), args);
    }
}
//void VirtualBox::zoom(qreal factor)
//{
//    QScriptValue zoom = m_engine->globalObject().property("zoom");
//    if (zoom.isFunction()) {
//        QScriptValueList args;
//        args << factor;
//        zoom.call(QScriptValue(), args);
//    }

//    if (factor == 0) {
//        m_magnify->setShow(false);
//        m_magnify->setActive(false);
//        return ;
//    } else {
//        m_magnify->setShow(true);
//        m_magnify->setActive(true);
//    }
//    m_magnify->setRatio(factor);

//}

#include "virtual_box.moc"
