#include "render_item.h"

#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLFramebufferObject>
#include <QQuickWindow>
#include <qsgsimpletexturenode.h>
#include <QSize>
#include <QTimer>

class RenderThread: public QQuickFramebufferObject::Renderer
{
public:
    RenderThread(QDomNode root):\
        m_render_base(0){

        QDomNode replace;
        m_root_config.clear ();
        replace = m_root_config.createElement ("replace");
        replace = root.cloneNode ();
        m_root_config.appendChild (replace);

        qDebug() << m_root_config.toString ();
        if (newRender() == false) {
            qWarning() << "newRender false";
            return;
        }
        initParams();
        m_render_base->initialize();
    }

    ~RenderThread() {}

    void render() {
        m_render_base->paint ();
        update();
    }


    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size){
        QOpenGLFramebufferObjectFormat format;

        format.setAttachment (QOpenGLFramebufferObject::CombinedDepthStencil);
        format.setSamples (4);

        return new QOpenGLFramebufferObject(size, format);
    }

    void synchronize(QQuickFramebufferObject *item) {
        RenderItem *_parent = qobject_cast<RenderItem *>(item);

        if (!_parent->paramsNode ().isNull ()) {
            setParams(_parent->paramsNode ());
        }

        if (!_parent->dataNode ().isEmpty ()) {
            addData (0, \
                     _parent->dataNode ().data (), \
                     _parent->dataNode ().size ());
        }

    }

    bool newRender() {
        QDomNode node = m_root_config.documentElement ().firstChildElement ("mode");
        QString  mode;
        QSize    size;

        if (node.isElement ()) {
            mode = node.toElement ().attribute ("value");
        }
        node = m_root_config.documentElement ().firstChildElement ("size");

        if (node.isElement ()) {
            int width = node.toElement ().attribute ("width", "768").toInt ();
            int height= node.toElement ().attribute ("height","512").toInt ();
            size = QSize(width, height);
        }

        if (mode == "B" || mode == "b") {
            m_render_base = new RenderB(size);
            return true;
        }

        return false;

    }
    void initParams() {
        QDomNode node = m_root_config.firstChildElement ();

        while (!node.isNull ()) {
            setParams(node);
            node = node.nextSiblingElement ();
        }
    }

    void addData  ( int id, char *buf, int size){
        if (m_render_base) m_render_base->addData (id, buf, size);
    }

    void setParams( QDomNode node) {
        if (node.isElement ()) {
            if (m_render_base) {
                if (node.isElement () && node.toElement ().tagName () == "resource") {
                    m_render_base->loadResure (node);
                }else if (node.isElement () && node.toElement ().tagName () == "probe") {
                    m_render_base->setGeoProbe (node);
                }else if (node.isElement () && node.toElement ().tagName () == "scan") {
                    m_render_base->setScan(node);
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

    RenderBase                 *m_render_base;
    QDomDocument               m_root_config;

    QString                    m_mode_string;
    QSize                      m_size;
};



RenderItem::RenderItem(QDomNode root, QQuickItem *parent): \
    QQuickFramebufferObject (parent)
{
    QDomNode replace;

    if (root.toElement ().tagName () != QString("RenderItem")) {
        qWarning() << __FUNCTION__ << "DomNode format's err";
        return;
    }

    int width = root.toElement ().attribute ("width").toInt ();
    int height= root.toElement ().attribute ("height").toInt ();

    setSize (QSize(width, height));

    m_config_doc.clear ();
    replace = m_config_doc.createElement ("replace");
    replace = root.cloneNode ();
    m_config_doc.appendChild (replace);

    /* -- test -- */
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(addD ()));

    timer->start(10);
}

QQuickFramebufferObject::Renderer *RenderItem::createRenderer () const
{
    QDomNode root = m_config_doc.documentElement ();
    return new RenderThread(root.firstChildElement ("render"));
}

QDomNode RenderItem::paramsNode() const
{
    return m_render_params;
}

QByteArray RenderItem::dataNode () const
{
    return m_render_data;
}

/*----test----*/

void RenderItem::addD ()
{
    m_render_data.resize ( 768*512);
    static int i = 0;
    m_render_data.fill (i);
    i = (i+1)%255;

    update();
}

void RenderItem::addData (QByteArray a)
{
    m_render_data = a;
    update();
}

void RenderItem::setParams (QDomNode root)
{
    m_render_params = root;
    update();
}

