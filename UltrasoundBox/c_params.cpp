#include "c_params.h"
#include <QDomNode>
#include <QDebug>

#define MODULE_INFO "CParams"

class CParamsPrivate {
public:
    CParamsPrivate(QScriptEngine *engine, QDomNode node, CParams *qptr): \
        engine(engine), q_ptr(qptr) {
        QDomNode root;
        root = node.firstChildElement(MODULE_INFO);
        if (root.isNull()) {
        } else {
        }
    }
    ~CParamsPrivate() {

    }
    QScriptEngine   *engine;
    CParams     *q_ptr;


};

CParams::CParams(QScriptEngine *engine, QDomNode &node) : \
    m_dptr(new CParamsPrivate(engine, node, this))
{
    setObjectName (node.toElement ().tagName ());
    engine->globalObject ().setProperty (MODULE_INFO, engine->newQObject (this));
    if (engine->hasUncaughtException ()){
        qWarning() << "Error:" << __FILE__ << __LINE__ << "CParams " << engine->uncaughtExceptionLineNumber () << " " << engine->uncaughtException ().toString ();
    }
}

CParams:: ~CParams() {
    if (m_dptr) {
        delete m_dptr;
        m_dptr = 0;
    }
}
