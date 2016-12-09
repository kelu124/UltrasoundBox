#include "d_params.h"
#include <QDomNode>
#include <QDebug>

#define MODULE_INFO "DParams"

#ifndef DPARAMSDEFINE
enum DParamsEnum{
	D_Ver = 0,	//版本号【1~】
	D_prf,	//脉冲重复频率，单位Hz
	D_wallfilter_id, //壁滤波ID 目前固定为0 ×
	D_invert,	//频谱翻转，【0~1】
	D_baseline,	//基线调节，基线正中为0，范围【-127~127】
	D_dyn_id,		//频谱的动态范围，【0~15】，目前固定为0 ×
	D_triplex,	//三同步（BD、BCD）模式，【0~1】，0为同步，1为单D
	D_audiovolume,//频谱音量大小，【0~255】，0为关闭频谱
	D_resolution,	//频谱分辨率，【0~4】目前固定为0 ×
	D_imgopt_id,	//频谱的图像处理【0~3】
	D_linepersec,//传1秒钟需要产生多少条pw线对应D_SPEED，【1~250】
	D_greymap_id,	//频谱的灰阶条，目前不用【0-7】=0 ×
	D_tintmap_id,	//频谱的伪彩条，目前不用【0-7】=0 ×
	D_autotrace,	//自动包络，目前不用【0~1】 ×
	D_steer,	//偏转扫描，目前不用 ×
	D_flowangle,//血流角度，目前不用 ×
	D_img_w,		//图像宽，建议768
	D_img_h,		//图像高，建议256
	D_pw_roi,	//PW成像区域，目前不用 ×
	D_Reversed
};
#endif
class DParamsPrivate {
public:
    DParamsPrivate(QScriptEngine *engine, QDomNode node, DParams *qptr): \
        engine(engine), q_ptr(qptr) {
        root = node.firstChildElement(MODULE_INFO);
        if (root.isNull()) {
            QDomDocument doc = node.toDocument();
            root = doc.createElement(MODULE_INFO);

            audio = doc.createElement("audio");
            width = doc.createElement("width");
            height = doc.createElement("height");
            linepersec = doc.createElement("linepersec");
            prf	= doc.createElement("prf");
            ver = doc.createElement("ver");
            baseline = doc.createElement("baseline");
            autotrace = doc.createElement("autotrace");
            invert = doc.createElement("invert");
            optid = doc.createElement("optid");
            resolution = doc.createElement("resolution");
            triplex = doc.createElement("triplex");
			maxReplay = doc.createElement("maxReplay");

            root.appendChild(audio);
            root.appendChild(width);
            root.appendChild(height);
            root.appendChild(linepersec);
            root.appendChild(prf);
            root.appendChild(ver);
            root.appendChild(baseline);
            root.appendChild(autotrace);
            root.appendChild(invert);
            root.appendChild(optid);
            root.appendChild(resolution);
            root.appendChild(triplex);
			root.appendChild(maxReplay);

            node.appendChild(root);

            QString saudio = QString("%1").arg(255);
            audio.setAttribute("value", saudio);

            QString swidth = QString("%1").arg(256);
            width.setAttribute("value", swidth);

            QString sheight = QString("%1").arg(256);
            height.setAttribute("value", sheight);

            QString slinepersec = QString("%1").arg(70);
            linepersec.setAttribute("value", slinepersec);

            QString sprf = QString("%1").arg(7300);
            prf.setAttribute("value", sprf);

            QString sver = QString("%1").arg(1);
            ver.setAttribute("value", sver);

            QString sbaseline = QString("%1").arg(0);
            baseline.setAttribute("value", sbaseline);

            QString sautotrace = QString("%1").arg(0);
            autotrace.setAttribute("value", sautotrace);

            QString sinvert = QString("%1").arg(0);
            invert.setAttribute("value", sinvert);

            QString soptid = QString("%1").arg(1);
            optid.setAttribute("value", soptid);

            QString sresolution = QString("%1").arg(1);
            resolution.setAttribute("value", sresolution);

            QString striplex = QString("%1").arg(1);
            triplex.setAttribute("value", striplex);
			maxReplay.setAttribute("value", 512);
        } else {
            audio = root.firstChildElement("audio");
            width = root.firstChildElement("width");
            height = root.firstChildElement("height");
            linepersec = root.firstChildElement("linepersec");
            prf	= root.firstChildElement("prf");
            ver = root.firstChildElement("ver");
            baseline = root.firstChildElement("baseline");
            autotrace = root.firstChildElement("autotrace");
            invert = root.firstChildElement("invert");
            optid = root.firstChildElement("optid");
            resolution = root.firstChildElement("resolution");
            triplex = root.firstChildElement("triplex");
			maxReplay = root.firstChildElement("maxReplay");

        }
        currentMax = 0;
    }
    ~DParamsPrivate() {

    }

    int currentMax;
    QScriptEngine   *engine;
    DParams     *q_ptr;

    QDomNode    root;
    QDomElement audio;
    QDomElement width;
    QDomElement height;
    QDomElement linepersec;
    QDomElement prf;
    QDomElement ver;
    QDomElement baseline;
    QDomElement autotrace;
    QDomElement invert;
    QDomElement optid;
    QDomElement resolution;
    QDomElement triplex;
	QDomElement maxReplay;

};

DParams::DParams(QScriptEngine *engine, QDomNode &node) : \
    m_dptr(new DParamsPrivate(engine, node, this))
{
    setObjectName (node.toElement ().tagName ());
    engine->globalObject ().setProperty (MODULE_INFO, engine->newQObject (this));
    if (engine->hasUncaughtException ()){
        qWarning() << "Error:" << __FILE__ << __LINE__ << "DParams " << engine->uncaughtExceptionLineNumber () << " " << engine->uncaughtException ().toString ();
    }
}

DParams:: ~DParams() {
    if (m_dptr) {
        delete m_dptr;
        m_dptr = 0;
    }
}
int DParams::audio () const
{
    return m_dptr->audio.attribute("value", 0).toInt();
}
int DParams::getVBoxID()
{
    QScriptValue vbox = m_dptr->engine->globalObject().property("vbox");
    if (!vbox.isValid()) {
        qWarning() << "Error: " << __FILE__ << __LINE__ << "vbox is NUll";
        return -1;
    }

    int id = 0;
    QMetaObject::invokeMethod( vbox.toQObject(), "ID", Qt::DirectConnection, \
                               Q_RETURN_ARG(int, id));
    return id;
}
int DParams::SetDParams(int pos, int value)
{
    QScriptValue vbox = m_dptr->engine->globalObject().property("vbox");
    if (!vbox.isValid()) {
        qWarning() << "Error: " << __FILE__ << __LINE__ << "vbox is NUll";
        return -1;
    }
    QMetaObject::invokeMethod(vbox.toQObject(), "SetDParams", Q_ARG(int, pos), Q_ARG(int, value));
    return 0;
}
int DParams::SetDParams(int pos, float value)
{
    QScriptValue vbox = m_dptr->engine->globalObject().property("vbox");
    if (!vbox.isValid()) {
        qWarning() << "Error: " << __FILE__ << __LINE__ << "vbox is NUll";
        return -1;
    }
    QMetaObject::invokeMethod(vbox.toQObject(), "SetDParams", Q_ARG(int, pos), Q_ARG(float, value));
    return 0;
}
void DParams::setAudio (int _value)
{

    SetDParams(D_audiovolume, _value);


    if ( audio() == _value)
        return;
    m_dptr->audio.setAttribute("value", _value);

    emit audioChanged();
}
int DParams::width () const
{
    return m_dptr->width.attribute("value", 0).toInt();
}
void DParams::setWidth (int _value)
{
    SetDParams(D_img_w, _value);
    if ( width() == _value)
        return;
    m_dptr->width.setAttribute("value", _value);

    emit widthChanged();
}
int DParams::height () const
{
    return m_dptr->height.attribute("value", 0).toInt();
}
void DParams::setHeight (int _value)
{
    SetDParams(D_img_h, _value);
    if ( height() == _value)
        return;
    m_dptr->height.setAttribute("value", _value);

    emit heightChanged();
}
float DParams::linepersec () const
{
    return m_dptr->linepersec.attribute("value", 0).toFloat();
}
void DParams::setLinepersec (float _value)
{
    SetDParams(D_linepersec, _value);

    if ( linepersec() == _value)
        return;
    m_dptr->linepersec.setAttribute("value", _value);

    emit linepersecChanged();
}
int DParams::prf () const
{
    return m_dptr->prf.attribute("value", 0).toInt();
}
void DParams::setPrf (int _value)
{
    SetDParams(D_prf, _value);
    if ( prf() == _value)
        return;
    m_dptr->prf.setAttribute("value", _value);

    emit prfChanged();
}
int DParams::ver () const
{
    return m_dptr->ver.attribute("value", 0).toInt();
}
void DParams::setVer (int _value)
{
    SetDParams(D_Ver, _value);
    if ( ver() == _value)
        return;
    m_dptr->ver.setAttribute("value", _value);

    emit verChanged();
}
int DParams::baseline () const
{
    return m_dptr->baseline.attribute("value", QString("0")).toInt();
}
void DParams::setBaseline (int _value)
{
    SetDParams(D_baseline, _value);
    m_dptr->baseline.setAttribute("value", _value);

    emit baselineChanged();
}
int DParams::autotrace () const
{
    return m_dptr->autotrace.attribute("value", 0).toInt();
}
void DParams::setAutotrace (int _value)
{
    SetDParams(D_autotrace, _value);
    if ( autotrace() == _value)
        return;
    m_dptr->autotrace.setAttribute("value", _value);


    emit autotraceChanged();
}
int DParams::invert () const
{
    return m_dptr->invert.attribute("value", 0).toInt();
}
void DParams::setInvert (int _value)
{
    if ( invert() == _value)
        return;
    m_dptr->invert.setAttribute("value", _value);

    SetDParams(D_invert, _value);
    emit invertChanged();
}
int DParams::optid () const
{
    return m_dptr->optid.attribute("value", 0).toInt();
}
void DParams::setOptid (int _value)
{
    if ( optid() == _value)
        return;
    m_dptr->optid.setAttribute("value", _value);
    SetDParams(D_imgopt_id, _value);
    emit optidChanged();
}
int DParams::resolution () const
{
    return m_dptr->resolution.attribute("value", 0).toInt();
}
void DParams::setResolution (int _value)
{
    if ( resolution() == _value)
        return;
    m_dptr->resolution.setAttribute("value", _value);

    SetDParams(D_resolution, _value);
    emit resolutionChanged();
}
int DParams::triplex () const
{
    return m_dptr->triplex.attribute("value", 0).toInt();
}
void DParams::setTriplex (int _value)
{
    if ( triplex() == _value)
        return;
    m_dptr->triplex.setAttribute("value", _value);
    SetDParams(D_triplex, _value);
    emit triplexChanged();
}
int DParams::maxReplay() const
{
	return m_dptr->maxReplay.attribute("value").toInt();
}
void DParams::setMaxReplay(int v)
{
	if (v == maxReplay()) return ;
	m_dptr->maxReplay.setAttribute("value", v);

	emit maxReplayChanged();
}
int DParams::currentMax() const
{
    return m_dptr->currentMax;
}
void DParams::setCurrentMax(int v)
{
    if (v == currentMax()) return ;
    m_dptr->currentMax = v;
    emit currentMaxChanged();
}
