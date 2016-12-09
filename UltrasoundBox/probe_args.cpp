#include "probe_args.h"
#include <QDomDocument>
#include <QScriptEngine>
#include <QFile>
#include <QDebug>


ProbeArgs::ProbeArgs(QDomDocument *doc, QScriptEngine *engine): \
   m_engine(engine), m_doc(doc)
{
    if (!doc) {
        qWarning() << "ProbeArgs Error " << __FILE__ << __LINE__ << __FUNCTION__;
        qWarning() << engine->uncaughtExceptionLineNumber() << engine->uncaughtException().toString();
        return;
    }
    engine->globalObject().setProperty("ProbeArgs", engine->newQObject(this));

    m_type     = doc->documentElement().firstChildElement("type");
    m_element  = doc->documentElement().firstChildElement("element");
    m_interval = doc->documentElement().firstChildElement("interval");
    m_radius   = doc->documentElement().firstChildElement("radius");
    m_angle    = doc->documentElement().firstChildElement("angle");
    m_lineWidth= doc->documentElement().firstChildElement("lineWidth");

    m_wobbling = doc->documentElement().firstChildElement("wobbling");
    m_slice1   = doc->documentElement().firstChildElement("slice1");
    m_slice2   = doc->documentElement().firstChildElement("slice2");
    m_slice3   = doc->documentElement().firstChildElement("slice3");
    m_slice4   = doc->documentElement().firstChildElement("slice4");

}

ProbeArgs:: ~ProbeArgs() {
}

int ProbeArgs::element() const
{
    return m_element.attribute("value").toInt();
}

void ProbeArgs::setElement(int value)
{
    if ( value == element()){
        return;
    }

    m_element.setAttribute("value", value);

    emit  elementChanged();
}

float ProbeArgs::interval() const
{
    return m_interval.attribute("value").toFloat();
}

void ProbeArgs::setInterval(float value)
{
    m_interval.setAttribute("value", value);

    emit intervalChanged();
}

float ProbeArgs::radius() const
{
    return m_radius.attribute("value").toFloat();
}

void ProbeArgs::setRadius(float value)
{
    m_radius.setAttribute("value", value);

    emit radiusChanged();
}

double ProbeArgs::angle() const
{
	return m_angle.attribute("value").toDouble();
}

void ProbeArgs::setAngle(double value)
{
    m_angle.setAttribute("value", value);

    emit angleChanged();
}

float ProbeArgs::lineWidth() const
{
    return m_lineWidth.attribute("value").toFloat();
}

void ProbeArgs::setLineWidth(float value)
{
    m_lineWidth.setAttribute("value", value);

    emit lineWidthChanged();
}
int ProbeArgs::type() const
{
    //    qDebug() << m_type.attribute("value").toInt();
    return m_type.attribute("value").toInt();
}

void ProbeArgs::setType(int value)
{
    if (value == type()) {
        return;
    }

    m_type.setAttribute("value", value);

    emit typeChanged();
}

float ProbeArgs::wobbling()const
{
    return m_wobbling.attribute("value").toFloat();
}

void ProbeArgs::setWobbling(float value)
{
    m_wobbling.setAttribute("value", value);

    emit wobblingChanged();
}

int ProbeArgs::slice1() const
{
    return m_slice1.attribute("value").toInt();
}

void ProbeArgs::setSlice1(int value)
{
    if (slice1() == value) {
        return;
    }

    m_slice1.setAttribute("value", value);
    emit slice1Changed();
}

int ProbeArgs::slice2() const
{
    return m_slice2.attribute("value").toInt();
}

void ProbeArgs::setSlice2(int value)
{
    if (slice2() == value) {
        return;
    }

    m_slice2.setAttribute("value", value);
    emit slice2Changed();
}

int ProbeArgs::slice3() const
{
    return m_slice3.attribute("value").toInt();
}

void ProbeArgs::setSlice3(int value)
{
    if (slice3() == value) {
        return;
    }

    m_slice3.setAttribute("value", value);
    emit slice3Changed();
}

int ProbeArgs::slice4() const
{
    return m_slice4.attribute("value").toInt();
}

void ProbeArgs::setSlice4(int value)
{
    if (slice4() == value) {
        return;
    }

    m_slice4.setAttribute("value", value);
    emit slice4Changed();
}
