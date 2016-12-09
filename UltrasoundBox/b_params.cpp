#include "b_params.h"
#include <QDomNode>
#include <QDebug>

#define MODULE_INFO "BParams"

class BParamsPrivate {
public:
    BParamsPrivate(QScriptEngine *engine, QDomNode node, BParams *qptr): \
        engine(engine), q_ptr(qptr) {
        QDomNode root;
        root = node.firstChildElement(MODULE_INFO);
        if (root.isNull()) {
        } else {
        }
    }
    ~BParamsPrivate() {

    }
    QScriptEngine   *engine;
    BParams     *q_ptr;


};

BParams::BParams(QScriptEngine *engine, QDomNode &node) : \
    m_dptr(new BParamsPrivate(engine, node, this))
{
    setObjectName (node.toElement ().tagName ());
    engine->globalObject ().setProperty (MODULE_INFO, engine->newQObject (this));
    if (engine->hasUncaughtException ()){
        qWarning() << "Error:" << __FILE__ << __LINE__ << "BParams " << engine->uncaughtExceptionLineNumber () << " " << engine->uncaughtException ().toString ();
    }
}

BParams:: ~BParams() {
    if (m_dptr) {
        delete m_dptr;
        m_dptr = 0;
    }
}

void BParams::upParams ()
{

}
