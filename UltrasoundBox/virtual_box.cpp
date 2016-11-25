#include "virtual_box.h"

#include <QDebug>
#include <QString>
#include <QGuiApplication>
#include <QQuickWindow>

#include "render_base.h"
#include "render_b.h"

#include "eplugins_manager.h"
#include <qmath.h>
#include <QMutex>

#include <QOffscreenSurface>
#include <QOpenGLFramebufferObjectFormat>
#include <QSGSimpleTextureNode>
#if 0
QList<RenderThread *> RenderItem::threads;

class RenderThread : public QThread
{
    Q_OBJECT
public:
    /*
        <render>
            <mode  value="B">
                <B x= "-1" y = "-1" w = "2" h = "2"/>
            </mode>
            <probe name="R10" icon="res/R10.png" hard="16" soft="3" type="Convex" element="128" interval="1.175"
radius="1.0" Angle="150.4"/>
            <size width=768 height=512/>
        </render>

    */
    RenderThread(QDomNode root)
        : surface(0)
        , context(0)
        , m_renderFbo(0)
        , m_displayFbo(0)
        , m_size(QSize(0,0))
    {

        QDomNode node;

        m_render = 0;
        /* etc.. <mode value = "B" />*/
        node = root.firstChildElement (QString("mode"));
        if (node.isNull ()) {
            qWarning() << __FILE__ << __FUNCTION__ << "mode not found";
            return;
        }
        m_mode_string = node.toElement().attribute("value");

        /* etc.. <size width = "768 height = "512" */
        node = root.firstChildElement (QString("size"));
        if (node.isNull ()) {
            qWarning() << __FILE__ << __FUNCTION__ << "size not found";
            return;
        }
        if (node.isElement ()) {
            m_size.setWidth  (node.toElement ().attribute ("width",  "768").toInt ());
            m_size.setHeight (node.toElement ().attribute ("height", "512").toInt ());
        }

        m_root_config = root;

        RenderItem::threads << this;
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
            m_displayFbo = new QOpenGLFramebufferObject (m_size, format);

            if (m_mode_string == "B") {
                m_render = new RenderB(m_size);
                initParams();
            }

            m_render->initialize();
        }

        m_renderFbo->bind();
        context->functions ()->glViewport (0, 0, m_size.width(), m_size.height());

        m_render->paint();

        context->functions()->glFlush();

        m_renderFbo->bindDefault();
        qSwap(m_renderFbo, m_displayFbo);

        emit textureReady(static_cast<int>(m_displayFbo->texture()), m_size);
    }

    void shutDown()
    {
        context->makeCurrent(surface);
        delete m_renderFbo;
        delete m_displayFbo;
        delete m_render;
        context->doneCurrent();
        delete context;

        // schedule this to be deleted only after we're done cleaning up
        surface->deleteLater();

        // Stop event processing, move the thread to GUI and make sure it is deleted.
        exit();
        moveToThread(QGuiApplication::instance()->thread());
    }

    void setMap   ( int id, unsigned char *map, int size ){
        if (m_render) m_render->setMap (id, map, size);

    }

    void addData  ( int id, unsigned char *buf, int size){
        if (m_render) m_render->addData (id, buf, size);
    }

    void setScan  ( QDomNode node){
        if (m_render) m_render->setScan (node);
    }

    void setParams( QDomNode node) {
        if (node.isElement ()) {
            if (m_render) {
                if (node.isElement () && node.toElement ().tagName () == "resource") {
                    m_render->loadResure (node);
                }else if (node.isElement () && node.toElement ().tagName () == "probe") {
                    m_render->setGeoProbe (node);
                }else if (node.isElement () && node.toElement ().tagName () == "scan") {
                    m_render->setScan(node);
                }
            }else {
                QString name = node.toElement ().tagName ();

                QDomNode target = m_root_config.firstChildElement (name);
                if (!target.isNull ()) {
                    QDomNode parent = target.parentNode ();

                    parent.replaceChild (node.cloneNode (), target);
                }
            }
        }
    }

    void initParams() {
        QDomNode node = m_root_config.firstChildElement ();

        while (!node.isNull ()) {
            setParams(node);
            node = node.nextSiblingElement ();
        }
    }

signals:
    void textureReady(int id, const QSize &size);

private:
    QOpenGLFramebufferObject *m_renderFbo;
    QOpenGLFramebufferObject *m_displayFbo;

    RenderBase                *m_render;

    QDomNode                   m_root_config;
    QString                    m_mode_string;
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

    void newTexture(int id, const QSize &size) {
        m_mutex.lock();
        m_id = id;
        m_size = size;
        m_mutex.unlock();

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

            m_texture = m_window->createTextureFromId((uint)newId, size);
            setTexture(m_texture);

            markDirty(DirtyMaterial);

            // This will notify the rendering thread that the texture is now being rendered
            // and it can start rendering to the other one.
            emit textureInUse();
        }
    }

private:

    QMutex m_mutex;
    int m_id;

    QSize m_size;

    QSGTexture   *m_texture;
    QQuickWindow *m_window;

};

/*
    <RenderItem>
        <render>
            ...
        <render>
    </RenderItem>
*/

QTimer *timer1;
RenderItem::RenderItem(QDomNode root, QQuickItem *parent)
    : QQuickItem(parent)
    , m_renderThread(0)
{
    setFlag(ItemHasContents, true);

    if (root.toElement ().tagName () != QString("RenderItem")) {
        qWarning() << __FUNCTION__ << "DomNode format's err";
        return;
    }

    int width = root.toElement ().attribute ("width").toInt ();
    int height= root.toElement ().attribute ("height").toInt ();

    setSize(QSize(width,height));

    QDomNode d = m_config_document.createElement ("replace");
    d = root.cloneNode ();
    m_config_document.appendChild (d);

    QDomNode node = m_config_document.documentElement ().firstChildElement ("render");

    m_renderThread = new RenderThread(node);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(addD ()));

    timer->start (1);

    timer1 = new QTimer(this);
    connect(timer1, SIGNAL(timeout()), this, SLOT(sss ()));

    timer1->start (5000);
}

RenderItem::~RenderItem()
{

}

unsigned char gbuf[768*512];

void RenderItem::sss()
{
//        timer1->stop ();
//        this->stopRenderThread ();
}

void RenderItem::addD ()
{
    static int tmp = 0;
    for (int  i = 0; i < 768*512; i++) {
        gbuf[i] = tmp;
    }

    tmp = (tmp+1) % 255;
    addData (0, (unsigned char *)gbuf,  768*512);
}

void RenderItem::startRenderThread ()
{

}

void RenderItem::stopRenderThread ()
{
    connect(window(), &QQuickWindow::afterRendering, m_renderThread, &RenderThread::shutDown, Qt::DirectConnection);
}

void RenderItem::ready()
{
    m_renderThread->surface = new QOffscreenSurface();
    m_renderThread->surface->setFormat(m_renderThread->context->format());
    m_renderThread->surface->create();

    m_renderThread->moveToThread(m_renderThread);

    connect(window(), &QQuickWindow::sceneGraphInvalidated, m_renderThread, &RenderThread::shutDown, Qt::QueuedConnection);

    connect(this, SIGNAL(addData(int, unsigned char *, int)),m_renderThread, SLOT(addData(int, unsigned char *, int)));
    connect(this, SIGNAL(setParams(QDomNode)),               m_renderThread, SLOT(setParams(QDomNode)));

    m_renderThread->start();
    update();
}

QSGNode *RenderItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    qDebug() << __FUNCTION__;
    TextureNode *node = static_cast<TextureNode *>(oldNode);

    if (!m_renderThread->context) {
        QOpenGLContext *current = window()->openglContext();

        current->doneCurrent();

        m_renderThread->context = new QOpenGLContext();
        m_renderThread->context->setFormat(current->format());
        m_renderThread->context->setShareContext(current);
        m_renderThread->context->create();
        m_renderThread->context->moveToThread(m_renderThread);

        current->makeCurrent(window());
        QMetaObject::invokeMethod(this, "ready");
        return 0;
    }

    if (!node) {
        node = new TextureNode(window());

        connect(m_renderThread, &RenderThread::textureReady, node, &TextureNode::newTexture, Qt::DirectConnection);
        connect(node, &TextureNode::pendingNewTexture, window(), &QQuickWindow::update, Qt::QueuedConnection);
        connect(window(), &QQuickWindow::beforeRendering, node, &TextureNode::prepareNode, Qt::DirectConnection);
        connect(node, &TextureNode::textureInUse, m_renderThread, &RenderThread::renderNext, Qt::QueuedConnection);

        // Get the production of FBO textures started..
        QMetaObject::invokeMethod(m_renderThread, "renderNext", Qt::QueuedConnection);
    }

    node->setRect(boundingRect());

    return node;
}
#include "virtual_box.moc"
#endif
