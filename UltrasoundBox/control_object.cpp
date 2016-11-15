#include "control_object.h"

#include <QDebug>

ControlObject::ControlObject(QDomNode node, int value, int reset, QObject *parent) : QObject(parent)
{
    QString name = node.toElement().attribute("name");
    node.toElement().setAttribute("value", value);
    node.toElement().setAttribute("reset", reset);
    setObjectName(name);
}

int ControlObject::value() const{
    return m_node.toElement().attribute("value").toInt();
}

void ControlObject::setValue(int value)
{
    if (m_node.isNull() || value == m_node.toElement().attribute("value").toInt()) {
        return;
    }

    m_node.toElement().setAttribute("value", value);

    emit valueChanged();

    if (m_node.toElement().attribute("reset").toInt()) {
        emit resetChanged();
    }
}

int ControlObject::reset() const
{
    return m_node.toElement().attribute("reset").toInt();
}

QString ControlObject::name() const {
    return objectName();
}

void ControlObject::updateNode (QDomNode node)
{
    int value = node.toElement ().attribute ("value", "0").toInt ();
//    int reset = node.toElement ().attribute ("reset", "0").toInt ();

    if (value != m_node.toElement ().attribute ("value", "0").toInt ()) {
        m_node = node;
        emit valueChanged();
    }
}

Control::Control(QString file)
{
    Q_UNUSED (file);
}

Control::Control(QDomDocument *doc)
{
    m_hash_controls.clear ();
    m_engine = 0;
    m_doc    = 0;

    QDomNode node = doc->documentElement().firstChildElement();

    while (!node.isNull()) {
        if (node.isElement()) {
            QDomElement e = node.toElement();
            int value = e.attribute("value").toInt();
            int reset = e.attribute ("reset").toInt ();

            ControlObject *obj = new ControlObject(e, value, reset);
            m_hash_controls.insert(obj->name (), obj);
        }
        node = node.nextSiblingElement();
    }

    m_doc = doc;
}

Control::~Control ()
{
    engineUnLinked();
    qDeleteAll(m_hash_controls);
    m_hash_controls.clear ();
}

void Control::engineLinked (QScriptEngine *engine)
{
    if (m_engine) engineUnLinked ();

    QHashIterator<QString, ControlObject *> i(m_hash_controls);
    while (i.hasNext()) {
        i.next();
        engine->globalObject().setProperty(i.key(),  engine->newQObject(i.value()));
    }

    m_engine = engine;
}

void Control::engineUnLinked ()
{
    QHashIterator<QString, ControlObject *> i(m_hash_controls);
    while (i.hasNext()) {
        i.next();
        m_engine->globalObject().setProperty(i.key(),  QScriptValue());
    }

    m_engine = 0;
}

void Control::updateData (QDomDocument *doc)
{
    Q_ASSERT (doc);

    QDomNode node = doc->documentElement().firstChildElement();

    while (!node.isNull()) {
        if (node.isElement()) {
            QDomElement e = node.toElement();
//            int value = e.attribute("value").toInt();
//            int reset = e.attribute ("reset").toInt ();

            m_hash_controls[e.tagName ()]->updateNode (e);
//            m_hash_controls[e.tagName ()]->setReset (reset);
//            m_hash_controls[e.tagName ()]->setValue (value);
        }
        node = node.nextSiblingElement();
    }

    m_doc = doc;
}

void Control::setObjectValue (QString name, int value)
{
    if (m_hash_controls.contains (name)) {
        m_hash_controls[name]->setValue (value);

        QScriptValue caller = m_engine->globalObject().property(name + "_callback");
        if (caller.isValid()) {
            QScriptValueList args;
            args<< value;
            caller.call(QScriptValue(), args);
        } else {
            qWarning() << "failed found " << name << __FILE__ << __FUNCTION__;
        }
    }
}

int Control::objectValue(QString name)
{
    int ret = -1;

    if (m_hash_controls.contains (name)) {
        ret = m_hash_controls[name]->value();
    }

    return ret;
}
