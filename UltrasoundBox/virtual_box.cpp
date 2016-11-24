#include "virtual_box.h"

#include <QDebug>


#include <QString>

#include <QMutex>
#include <QThread>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <qsgsimpletexturenode.h>

#include <qopenglshaderprogram.h>
#include <qopenglfunctions.h>
#include <qopenglfunctions.h>
#include <QQuickWindow>
#include <QOpenGLTexture>
#include <QOpenGLShader>

#include <QDomNode>

#include <QDir>
#include <QQuickWindow>

#include "eplugins_manager.h"
#include <qmath.h>

#include <QMutex>
class RenderBase : protected QOpenGLFunctions
{
public:
    RenderBase() {}

    virtual ~RenderBase() {}
    virtual void initialize() {}
    virtual void paint() {}
    virtual void createGeometry() {}
    virtual void setMap   ( int, unsigned char *, int ) {}
    virtual void addData  ( int, unsigned char *, int ) {}
    virtual void setScan(QDomNode) {}

    void setViewPort (int vx, int vy, int vw, int vh);
    void setViewPort (QRect rect);

    void setGeoTrans (QDomNode);
    void setGeoProbe (QDomNode);

    QByteArray readFile(QString &);
    void loadResure(QDomNode);

    QMatrix4x4 mModelMatrix;
    QMatrix4x4 mViewMatrix;
    QMatrix4x4 mProjectionMatrix;

    QMatrix4x4 mMVPMatrix;
    QSize      mSize;

    QVector<QVector3D> colorsMapB;
    QVector<QVector3D> colorsMapC;
    QVector<QVector3D> colorsMapM;
    QVector<QVector3D> colorsMapPW;

    QVector<QVector3D> mVerticesB;
    QVector<QVector2D> mTexcoordsB;

    QVector<QVector3D> mVerticesC;
    QVector<QVector2D> mTexcoordsc;

    QVector<QVector3D> mVerticesM;
    QVector<QVector2D> mTexcoordsM;

    QVector<QVector3D> mVerticesPW;
    QVector<QVector2D> mTexcoordsPW;


    struct mapData {
        unsigned char b_colormaplist[8][256][3];
        unsigned char c_colormaplist[8][256][3];
    };

    struct st_img_trans{
        int            grayReverse;
        struct         mapData colormap;
    }m_img_trans;


    struct st_geo_trans{
        int        angle;
        int        x_turn;
        int        y_turn;
        QRectF     cutRect;
    }m_geo_trans;


    struct st_Probe {
        int   hard;
        int   soft;

        int   element;
        float interval;
        float radius;
        float angle;
        float linewidth;

        QString name;
        QString icon;
        QString type;

    }m_probe;

    QMutex      m_mutex;
};

QByteArray RenderBase::readFile (QString &n)
{
    QFile file(n);
    if (!file.open (QFile::ReadOnly)) {
        return QByteArray();
    }

    QByteArray data = file.readAll ();

    file.close ();
    return data;
}
void RenderBase::loadResure (QDomNode root)
{
    if (root.isNull ()) return;

    qDebug() << root.toElement ().tagName ();

    QDomNode node = root.firstChildElement ();

    while(!node.isNull ()) {
        if (node.isElement ()) {
            if (node.toElement ().tagName () == QString("b")) {
                QString name = node.toElement ().attribute ("value", QString("0"));
                QByteArray data = readFile (name);

                if (!data.isEmpty ()) {
                    for (int i = 0; i < 8; i++) {
                        for (int j = 0; j < 256; j++) {
                            char ch = data.at (i * 256 + j);
                            m_img_trans.colormap.b_colormaplist[i][j][0] = static_cast<unsigned char>(ch);
                            m_img_trans.colormap.b_colormaplist[i][j][1] = static_cast<unsigned char>(ch);
                            m_img_trans.colormap.b_colormaplist[i][j][2] = static_cast<unsigned char>(ch);
                        }
                    }
                }
            }else if (node.toElement ().tagName () == QString("c")){
                /*add you  code*/
            }else if (node.toElement ().tagName () == QString("d")) {
                /*add your code*/
            }else if (node.toElement ().tagName () == QString("m")) {
                /*add your code*/
            }
        }

        node = node.nextSiblingElement ();
    }

}

void RenderBase::setViewPort (int vx, int vy, int vw, int vh)
{
    Q_UNUSED(vx);
    Q_UNUSED(vy);
    mViewMatrix.setToIdentity();
    mViewMatrix.lookAt(QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0, 0.0f, -5.0f), QVector3D(0.0f, 1.0f, 0.0f));
    mViewMatrix.scale(mSize.width ()/vw, mSize.height ()/vh, 1.0);
}

void RenderBase::setViewPort (QRect rect)
{
    setViewPort(rect.x(), rect.y(), rect.width(), rect.height());
}


void RenderBase::setGeoTrans (QDomNode root)
{
    if (root.isNull ()) return;

    qDebug() << root.toElement ().tagName ();

    QDomNode node = root.firstChildElement ();

    while(!node.isNull ()) {
        if (node.isElement ()) {
            if (node.toElement ().tagName () == QString("rotate")) {
                int angle = node.toElement ().attribute ("value", QString("0")).toInt ();
                if (angle == 0) {
                    mModelMatrix.setToIdentity ();
                }else {
                    mModelMatrix.rotate (angle, 0.0f, 0.0f, 1.0f);
                }
                m_geo_trans.angle = angle;
            }else if (node.toElement ().tagName () == QString("turnUD")){
                int v = node.toElement ().attribute ("value", QString("0")).toInt();
                if (v == 0) {
                    m_geo_trans.y_turn = 1;
                    mModelMatrix.setToIdentity ();
                }else {
                    m_geo_trans.y_turn = -1;
                    mModelMatrix.rotate (180, 1, 0, 0);
                }
            }else if (node.toElement ().tagName () == QString("turnLR")) {
                int v = node.toElement ().attribute ("value", QString("0")).toInt ();

                if (v == 0) {
                    m_geo_trans.x_turn = 1;
                    mModelMatrix.setToIdentity ();
                }else {
                    m_geo_trans.x_turn = -1;
                    mModelMatrix.rotate (180, 1, 0, 0);
                }
            }else if (node.toElement ().tagName () == QString("size")) {
                int w = node.toElement ().attribute ("width", QString("0")).toInt ();
                int h = node.toElement ().attribute ("height", QString("0")).toInt ();

                mSize = QSize(w, h);
            }
        }

        node = node.nextSiblingElement ();
    }

}

void RenderBase::setGeoProbe (QDomNode node)
{
    if (node.isNull ()) {
        return;
    }

    QString probeType = node.toElement ().attribute ("type");

    if (probeType == QString("Convex")) {
        m_probe.name = node.toElement ().attribute ("name", "");
        m_probe.icon = node.toElement ().attribute ("icon", "");
        m_probe.hard = node.toElement ().attribute ("hard", "0").toInt ();
        m_probe.soft = node.toElement ().attribute ("soft", "").toInt ();
        m_probe.type = probeType;
        m_probe.element = node.toElement ().attribute ("element").toInt();
        m_probe.interval= node.toElement ().attribute ("interval", "0").toFloat ();
        m_probe.radius  = node.toElement ().attribute ("radius", "0").toFloat ();
        m_probe.angle   = node.toElement ().attribute ("angle", "0").toFloat ();
    }else if (probeType == QString("Line")) {
        m_probe.name = node.toElement ().attribute ("name", "");
        m_probe.icon = node.toElement ().attribute ("icon", "");
        m_probe.hard = node.toElement ().attribute ("hard", "0").toInt ();
        m_probe.soft = node.toElement ().attribute ("soft", "").toInt ();
        m_probe.type = probeType;
        m_probe.element  = node.toElement ().attribute ("element").toInt();
        m_probe.interval = node.toElement ().attribute ("interval", "0.0").toFloat ();
        m_probe.linewidth= node.toElement ().attribute ("lineWidth").toFloat ();
    }else {
        qDebug() << "not support probe";
    }

}


struct st_opengl_data {
    QOpenGLTexture    *texture;
    QVector<QVector3D> vertices;
    QVector<QVector2D> vercoords;
};

struct st_complex{
    struct st_opengl_data opengl_data_l;
    struct st_opengl_data opengl_data_m;
    struct st_opengl_data opengl_data_r;

};



class RenderB : public RenderBase
{
public:
    RenderB(QSize);
/*remmap*/
    void initialize();
    void paint();
    void createGeometry();

    void setMap  ( int, unsigned char *, int );
    void addData ( int, unsigned char *, int );
    void setScan (QDomNode);

private:
    void initShaders();
    void initData();

    void drawDSC();

    QOpenGLTexture *getTextures();

    QOpenGLShaderProgram	 program;

    int mMVPMatrixHandle;
    int mPositionHandle;
    int mTexCoordHandle;
    int mColorArrayhandle;

    struct st_scan{
        float depth;

        struct st_opengl_data mData;
    }m_scan;

    struct st_textureGeo{
        int     lines;
        int     samples;
    }m_texture_geo = {
        256,
        512
    };

    QByteArray      mByteArrayData;
};

RenderB::RenderB(QSize size) : program(0)
{
    mSize = size;
    mByteArrayData.resize (size.width () * size.height ());

    memset (mByteArrayData.data (),  0, size.width () * size.height ());

    colorsMapB.clear ();
    for (int i = 0; i < 256; i++) {
        colorsMapB << QVector3D(i, i, i);
    }

    m_texture_geo.lines   = 256;
    m_texture_geo.samples = 512;

    m_img_trans.grayReverse = 0;

}


void RenderB::initialize()
{
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    initShaders();
    initData();

    createGeometry ();
}

void RenderB::createGeometry ()
{
    mVerticesB  << QVector3D(-1, -1, 0.0f) \
                << QVector3D( 1, -1, 0.0f) \
                << QVector3D( 1,  1, 0.0f) \
                << QVector3D(-1,  1, 0.0f);

    mTexcoordsB << QVector2D (0 , 0) \
                 << QVector2D (1 , 0) \
                 << QVector2D (1,  0) \
                 << QVector2D (0,  1);
    mViewMatrix.setToIdentity ();
    mViewMatrix.lookAt (QVector3D (0.0f, 0.0f, 1.001f), QVector3D(0.0f, 0.0f, -5.0f), QVector3D(0.0f, 1.0f, 0.0f));

}

void RenderB::paint()
{

    mMVPMatrixHandle = program.uniformLocation("u_MVPMatrix");
    mPositionHandle  = program.attributeLocation("a_Position");
    mTexCoordHandle	 = program.attributeLocation("a_texCoord");
    mColorArrayhandle = program.uniformLocation("color_value");

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable( GL_DEPTH_TEST );
    glEnable(GL_TEXTURE_2D);

    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, m_scan.mData.texture->textureId ());

    m_scan.mData.texture->bind();
    program.bind();
    drawDSC();
    program.release();
    m_scan.mData.texture->release ();
}

void RenderB::setMap (int id, unsigned char *map, int size)
{
    Q_UNUSED (id);

    colorsMapB.clear ();
    for (int i = 0; i < size; i += 3) {
        colorsMapB << QVector3D(map[i], map[i+1], map[i+2]);
    }
}

void RenderB::initShaders()
{
//    initializeOpenGLFunctions();
//    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    program.addShaderFromSourceFile (QOpenGLShader::Vertex,   ":/glsl/shaderv.glsl");
    program.addShaderFromSourceFile (QOpenGLShader::Fragment, ":/glsl/shaderf.glsl");
    program.link();

    program.bindAttributeLocation("a_Position", 0);
    program.bindAttributeLocation("a_texCoord", 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    program.bind ();

}

QOpenGLTexture *RenderB::getTextures ()
{
    QOpenGLTexture *texture = new QOpenGLTexture(QOpenGLTexture::Target2D);

    texture->setSize(mSize.width (), mSize.height ());
    texture->setFormat(QOpenGLTexture::LuminanceFormat);
    texture->allocateStorage(QOpenGLTexture::Luminance, QOpenGLTexture::UInt8);

    texture->setMinificationFilter(QOpenGLTexture::Linear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

    texture->setWrapMode(QOpenGLTexture::Repeat);

    return texture;
}

void RenderB::initData()
{
    //set model matrix
    mModelMatrix.setToIdentity();
    //set view matrix
    mViewMatrix.setToIdentity();
    mViewMatrix.lookAt(QVector3D(0.0f, 0.0f, 1.00f), QVector3D(0.0f, 0.0f, -5.0f), QVector3D(0.0f, 1.0f, 0.0f));

    //set projection matrix
    float bottom=  -1.0f;
    float top	=	1.0f;
    float n		=	1.0f;
    float f		=	100.0f;
    //FIXME 1 投影矩阵 左右值取-1, 1，不要让图像被缩放
    mProjectionMatrix.setToIdentity();
    mProjectionMatrix.frustum(-1.0, 1.0, bottom, top, n, f);

    m_scan.mData.texture = getTextures ();
}

void RenderB::drawDSC()
{
    program.enableAttributeArray(mTexCoordHandle);
    program.setAttributeArray(mTexCoordHandle,  mTexcoordsB.constData());

    mMVPMatrix = mProjectionMatrix * mViewMatrix * mModelMatrix;

    program.setUniformValue (mMVPMatrixHandle, mMVPMatrix);
    program.setUniformValueArray (mColorArrayhandle, colorsMapB.constData (), colorsMapB.size ());

    program.setUniformValue ("grayReverse", static_cast<GLint>(m_img_trans.grayReverse));

    m_mutex.lock ();
    m_scan.mData.texture->setData (QOpenGLTexture::Luminance, QOpenGLTexture::UInt8, mByteArrayData.data ());
    m_mutex.unlock ();

    int texHandle = program.uniformLocation ("s_texture");
    program.setUniformValue (texHandle, 0);

    program.enableAttributeArray(mPositionHandle);
    program.setAttributeArray (mPositionHandle, mVerticesB.constData ());

    glDrawArrays (GL_TRIANGLE_FAN, 0, mVerticesB.size ());

    program.disableAttributeArray(mPositionHandle);
    program.disableAttributeArray(mTexCoordHandle);
}

void RenderB::setScan (QDomNode root)
{
    if (root.isNull ()) return;

    QDomNode node;

    node = root.firstChildElement ("depth");
    if (!node.isNull ()) {
        float value = node.toElement ().attribute("value", "1.0").toFloat ();
        m_scan.depth = value;
        createGeometry ();
    }
}

void RenderB::addData (int, unsigned char *data, int size)
{

//    m_mutex.lock ();
    mByteArrayData.resize (size);
    memcpy( mByteArrayData.data (), data, \
            static_cast<size_t>(sizeof(unsigned char) * static_cast<unsigned long>(size)));
//    m_mutex.unlock ();
}


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
