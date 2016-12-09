#include "m_params.h"
#include <QDomNode>
#include <QDebug>

#define MODULE_INFO "MParams"

class MParamsPrivate {
public:
    MParamsPrivate(QScriptEngine *engine, QDomNode node, MParams *qptr): \
        engine(engine), q_ptr(qptr) {
        root = node.firstChildElement(MODULE_INFO);
        if (root.isNull()) {
            QDomDocument doc = node.toDocument();
            root = doc.createElement(MODULE_INFO);
        } else {

        }

    }
    ~MParamsPrivate() {

    }

    QScriptEngine   *engine;
    MParams     *q_ptr;

    QDomNode    root;
};

MParams::MParams(QScriptEngine *engine, QDomNode &node) : \
    m_dptr(new MParamsPrivate(engine, node, this))
{
    setObjectName (node.toElement ().tagName ());
    engine->globalObject ().setProperty (MODULE_INFO, engine->newQObject (this));
    if (engine->hasUncaughtException ()){
        qWarning() << "Error:" << __FILE__ << __LINE__ << "MParams " << engine->uncaughtExceptionLineNumber () << " " << engine->uncaughtException ().toString ();
    }
}

MParams:: ~MParams() {
    if (m_dptr) {
        delete m_dptr;
        m_dptr = 0;
    }
}
