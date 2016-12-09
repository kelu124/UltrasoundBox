#include "layout_params.h"
#include <QDomNode>
#include <QDebug>

#define MODULE_INFO "LayoutParams"

class LayoutParamsPrivate {
public:
    LayoutParamsPrivate(QScriptEngine *engine, QDomNode node, LayoutParams *qptr): \
        engine(engine), q_ptr(qptr) {
        QDomNode root;
        root = node.firstChildElement(MODULE_INFO);

        if (root.isNull()) {
            QDomDocument doc = node.toDocument ();
            QDomNode root = doc.createElement (MODULE_INFO);

            echo = doc.createElement ("echo");
            echo.toElement ().setAttribute ("x", 0);
            echo.toElement ().setAttribute ("y", 0);
            echo.toElement ().setAttribute ("w", 0);
            echo.toElement ().setAttribute ("h", 0);

            image= doc.createElement ("image");
            image.toElement ().setAttribute ("x", 0);
            image.toElement ().setAttribute ("y", 0);
            image.toElement ().setAttribute ("w", 0);
            image.toElement ().setAttribute ("h", 0);

            root.appendChild (echo);
            root.appendChild (image);
        } else {
            echo = root.firstChildElement ("echo");
            image= root.firstChildElement ("image");
        }
    }
    ~LayoutParamsPrivate() {

    }


    QScriptEngine    *engine;
    LayoutParams     *q_ptr;

    QDomDocument     *m_doc;

    QDomNode echo, image;
};

LayoutParams::LayoutParams(QScriptEngine *engine, QDomNode &node) : \
    m_dptr(new LayoutParamsPrivate(engine, node, this))
{
    setObjectName (node.toElement ().tagName ());
    engine->globalObject ().setProperty (MODULE_INFO, engine->newQObject (this));
    if (engine->hasUncaughtException ()){
        qWarning() << "Error:" << __FILE__ << __LINE__ << "LayoutParams " << engine->uncaughtExceptionLineNumber () << " " << engine->uncaughtException ().toString ();
    }
}

LayoutParams:: ~LayoutParams() {
    if (m_dptr) {
        delete m_dptr;
        m_dptr = 0;
    }
}

int LayoutParams::echoX() const
{
    return m_dptr->echo.toElement ().attribute ("x", "0").toInt ();
}

int LayoutParams::echoY () const
{
    return m_dptr->echo.toElement ().attribute ("y", "0").toInt ();
}

int LayoutParams::echoW () const
{
    return m_dptr->echo.toElement ().attribute ("w", "0").toInt ();
}

int LayoutParams::echoH () const
{
    return m_dptr->echo.toElement ().attribute ("h", "0").toInt ();
}

void LayoutParams::setEchoX (int v)
{
    m_dptr->echo.toElement ().setAttribute ("x", v);
    emit echoXChanged();
}

void LayoutParams::setEchoY (int v)
{
    m_dptr->echo.toElement ().setAttribute ("y", v);
    emit echoYChanged();
}

void LayoutParams::setEchoW (int v)
{
    m_dptr->echo.toElement ().setAttribute ("w", v);
    emit echoWChanged();
}

void LayoutParams::setEchoH (int v)
{
    m_dptr->echo.toElement ().setAttribute ("h", v);
    emit echoHChanged();
}

int LayoutParams::imageX () const
{
    return m_dptr->image.toElement ().attribute ("x", "0").toInt ();
}

int LayoutParams::imageY () const
{
    return m_dptr->image.toElement ().attribute ("y", "0").toInt ();
}

int LayoutParams::imageW () const
{
    return m_dptr->image.toElement ().attribute ("w", "0").toInt ();
}

int LayoutParams::imageH () const
{
    return m_dptr->image.toElement ().attribute ("h", "0").toInt ();
}

void LayoutParams::setImageX (int v)
{
    m_dptr->image.toElement ().setAttribute ("x", v);
    emit imageXChanged();
}

void LayoutParams::setImageY (int v)
{
    m_dptr->image.toElement ().setAttribute ("y", v);
    emit imageYChanged();
}

void LayoutParams::setImageW (int v)
{
    m_dptr->image.toElement ().setAttribute ("w", v);
    emit imageWChanged ();
}

void LayoutParams::setImageH (int v)
{
    m_dptr->image.toElement ().setAttribute ("h", v);
    emit imageHChanged ();
}
