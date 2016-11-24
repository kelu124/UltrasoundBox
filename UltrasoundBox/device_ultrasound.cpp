#include "device_ultrasound.h"
#include <QHash>
#include <QObject>
#include <QString>
#include <QDomDocument>
#include <QFile>
#include <QDir>
#include <QBuffer>

#include <QThread>
#include <time.h>
#include <QMutexLocker>

#if 0
static void setFpgaAttrByAttr(QDomDocument *doc, QString attr, int value)
{
	QDomNode node = doc->documentElement().firstChildElement();
	while (!node.isNull()) {
		if (node.toElement().attribute("name") == attr) {
			node.toElement().setAttribute("value", value);
		}
		node = node.nextSiblingElement();
	}
}

FpgaControlObject::FpgaControlObject(QDomNode &node, int value, int reset): \
	m_node(node)
{
	QString name = node.toElement().attribute("name");
	node.toElement().setAttribute("value", value);
    node.toElement().setAttribute("reset", reset);
	setObjectName(name);
}

FpgaControlObject::~FpgaControlObject()
{

}

int FpgaControlObject::value() const{
	return m_node.toElement().attribute("value").toInt();
}

void FpgaControlObject::setValue(int value)
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

int FpgaControlObject::reset() const
{
    return m_node.toElement().attribute("reset").toInt();
}
QString FpgaControlObject::name() const {
	return objectName();
}


SoftControlObject::SoftControlObject(QDomNode &node, int value): \
	m_node(node)
{
	QString name = node.toElement().attribute("name");
	node.toElement().setAttribute("value", value);

	setObjectName(name);
}

SoftControlObject::~SoftControlObject() {

}

int SoftControlObject::value() const{
	return m_node.toElement().attribute("value").toInt();
}

void SoftControlObject::setValue(int value) {
	if (m_node.isNull() || value == m_node.toElement().attribute("value").toInt()) {
		return;
	}

	m_node.toElement().setAttribute("value", value);

	emit valueChanged();
}

QString SoftControlObject::name() const {
	return objectName();
}

SoftControl::SoftControl(QString xmlFileName) {
	Q_UNUSED(xmlFileName);
}

SoftControl::SoftControl(QDomDocument *doc):m_doc(doc) {
    if (m_doc == 0) {
        qWarning() << "SoftControl Error " << __FILE__ << __LINE__;
        return;
    }
    QDomNode node = m_doc->documentElement().firstChildElement();


	while (!node.isNull()) {
		if (node.isElement()) {
			QDomElement e = node.toElement();
			int value = e.attribute("value").toInt();

			SoftControlObject *obj = new SoftControlObject(node, value);
			m_hash_controls.insert(obj->objectName(), obj);
		}
		node = node.nextSiblingElement();
	}
}

SoftControl::~SoftControl()
{
	qDeleteAll(m_hash_controls);
	m_hash_controls.clear();
}

void SoftControl::changedSource(QDomDocument *doc) {
	Q_ASSERT(doc);

	m_doc = doc;
}

void SoftControl::changedUltrasound(Ultrasound *u){
	m_ultrasound = u;
}

void SoftControl::engineLinked(QScriptEngine *engine){
	Q_ASSERT(engine);

	QHashIterator<QString, SoftControlObject *> i(m_hash_controls);
	while (i.hasNext()) {
		i.next();
		engine->globalObject().setProperty(i.key(), engine->newQObject(i.value()));
	}

    m_engine = engine;
}

void SoftControl::setObjectValueFName(QString name, QString value)
{
	if (m_hash_controls.contains(name)) {
		m_hash_controls[name]->setValue(value.toInt());

		QScriptValue caller = m_engine->globalObject().property(name + "_callback");
		if (caller.isValid()) {
			QScriptValueList args;
			args<< value.toInt();
			caller.call(QScriptValue(), args);
		} else {
			qWarning() << "failed found " << name << __FILE__ << __FUNCTION__;
		}
    } else {
        qDebug() << "soft control has no "<< name;
    }
}

int SoftControl::objectValueFName(QString name)
{
	int ret = -1;
	if (m_hash_controls.contains(name)) {
		ret = m_hash_controls[name]->value();
	}
	return ret;
}

void SoftControl::objectValueChanged()
{
	QString name = sender()->objectName();

	QScriptValue caller = m_engine->globalObject().property(name + "_callback");
	if (caller.isValid()) {
		caller.call(QScriptValue());
	}else {
		qWarning() << "failed found " << name << __FILE__ << __FUNCTION__;
	}
}

FpgaControl::FpgaControl(QString xmlFileName) {
	Q_UNUSED(xmlFileName);
    m_reset_flag = false;
}

FpgaControl::FpgaControl(QDomDocument *doc) : m_doc(doc){
	if(m_doc == 0) {
		qWarning() << "FpgaControl Error" << __FILE__ << __LINE__;
		return;
	}
	m_engine = 0;
    m_reset_flag = false;

	QDomNode node = m_doc->documentElement().firstChildElement();

	while (!node.isNull()) {
		if (node.isElement()) {
			QDomElement e = node.toElement();
			int value = e.attribute("value").toInt();
            int flagResetBuffer = e.attribute("reset").toInt();

            FpgaControlObject *obj = new FpgaControlObject(node, value, flagResetBuffer);
			m_hash_controls.insert(obj->objectName(), obj);
			connect(obj, SIGNAL(valueChanged()), this, SLOT(objectValueChanged()));
            connect(obj, SIGNAL(resetChanged()), this, SLOT(resetChanged()));

		}
		node = node.nextSiblingElement();
	}
}

void FpgaControl::resetChanged()
{
    m_reset_flag = true;
}

FpgaControl::~FpgaControl() {
	qDeleteAll(m_hash_controls);
	m_hash_controls.clear();
}

void FpgaControl::objectValueChanged()
{
	QString name = sender()->objectName();

	QScriptValue caller = m_engine->globalObject().property(name + "_callback");
	if (caller.isValid()) {
		caller.call(QScriptValue());
	}else {
		qWarning() << "failed found " << name << __FILE__ << __FUNCTION__;
	}
}
void FpgaControl::setObjectValueFName(QString name, QString value)
{
	if (m_hash_controls.contains(name)) {
		m_hash_controls[name]->setValue(value.toInt());

		QScriptValue caller = m_engine->globalObject().property(name + "_callback");
		if (caller.isValid()) {
			caller.call(QScriptValue());
		}else {
			qWarning() << "failed found " << name << __FILE__ << __FUNCTION__;
		}
	}
}

int FpgaControl::objectValueFName(QString name)
{
	int ret = -1;
	if (m_hash_controls.contains(name)) {
		ret = m_hash_controls[name]->value();
	}
	return ret;
}

void FpgaControl::changedSource(QDomDocument *doc) {
	m_doc = doc;
}

void FpgaControl::changedUltrasound(Ultrasound *u){
	m_ultrasound = u;
}

void FpgaControl::engineLinked(QScriptEngine *engine){
	QHashIterator<QString, FpgaControlObject *> i(m_hash_controls);
	while (i.hasNext()) {
		i.next();
		engine->globalObject().setProperty(i.key(),  engine->newQObject(i.value()));
	}

	m_engine = engine;
}

static QDomNode readXmlFile2RootNode(QDomDocument *doc, QString fileName)
{

	QFile file(fileName);

	if (!file.open(QIODevice::ReadOnly)) {
		qDebug() << "Error:" << __FILE__ << __LINE__ << "DataFunctionXml(ReadXmlFile2RootNode): open file" << fileName << "failed,error:" << file.errorString();
		return QDomNode();
	}
	QString strError;
	int errorLin, errCol;
	if (!doc->setContent(&file, &strError, &errorLin, &errCol)) {
		qDebug() << "Error:" << __FILE__ << __LINE__ << "DataFunctionXml(ReadXmlFile2RootNode): Parse XML failed at line" << errorLin << ",col" << errCol << "," << strError;
		file.close();
		return QDomNode();
	}
	file.close();
	return doc->documentElement();
}

DeviceUltrasound *DeviceUltrasound::instance()
{
	static DeviceUltrasound *_this = NULL;

	if (! _this) {
		_this = new DeviceUltrasound();
	}

	return _this;
}

DeviceUltrasound::DeviceUltrasound()
    : Ultrasound("device")
    , m_linked(false)
{
	m_doc = 0;
	m_ref = 0;

//	if (!ret) {
//		m_linked = false;
//		qDebug() << __FILE__ << __FUNCTION__ << "__init__ Error ...";
//	}else m_linked = true;

}

QDomDocument *DeviceUltrasound::document()
{
	return m_doc;
}

int DeviceUltrasound::addRef()
{
	++m_ref;
	//	qDebug()<<__FILE__<<__FUNCTION__<<m_ref;
	if (m_ref == 1) {
		running();
		/*START device running*/
	}
	return m_ref;
}

int DeviceUltrasound::decRef()
{
	--m_ref;
	//	qDebug()<<__FILE__<<__FUNCTION__<<m_ref;
	if (m_ref <= 0) {
		freeze();
		/*STOP device running*/
	}

	return m_ref;
}

void DeviceUltrasound::modeB()
{
	//	qDebug()<<__FILE__<<__FUNCTION__<<m_ref;
//	if (m_device && m_linked && m_doc && m_ref != 0) {
//		setFpgaAttrByAttr(m_doc, "B", 1);
//		if (m_ref == 1) {
//			setFpgaAttrByAttr(m_doc, "C", 0);
//			setFpgaAttrByAttr(m_doc, "D", 0);
//			setFpgaAttrByAttr(m_doc, "M", 0);
//		}
//		updateCtrlTable(true);
//	} else {
//		qDebug()<<__FILE__<<__FUNCTION__<<"device cann't control";
//	}
}

void DeviceUltrasound::modeC()
{
	//	qDebug()<<__FILE__<<__FUNCTION__<<m_ref;
//	if (m_device && m_linked && m_doc && m_ref != 0) {
//		setFpgaAttrByAttr(m_doc, "C", 1);
//		setFpgaAttrByAttr(m_doc, "M", 0);
//		if (m_ref == 1) {
//			setFpgaAttrByAttr(m_doc, "B", 1);
//			setFpgaAttrByAttr(m_doc, "D", 0);
//			setFpgaAttrByAttr(m_doc, "M", 0);
//		}
//		updateCtrlTable(true);
//	} else {
//		qDebug()<<__FILE__<<__FUNCTION__<<"device cann't control";
//	}
}

void DeviceUltrasound::modeD()
{
	//	qDebug()<<__FILE__<<__FUNCTION__<<m_ref;
//	if (m_device && m_linked && m_doc && m_ref != 0) {
//		setFpgaAttrByAttr(m_doc, "D", 1);
//		setFpgaAttrByAttr(m_doc, "M", 0);
//		if (m_ref == 1) {
//			setFpgaAttrByAttr(m_doc, "C", 1);
//			setFpgaAttrByAttr(m_doc, "B", 1);
//			setFpgaAttrByAttr(m_doc, "M", 0);
//		}
//		updateCtrlTable(true);
//	} else {
//		qDebug()<<__FILE__<<__FUNCTION__<<"device cann't control";
//	}
}

void DeviceUltrasound::modeM()
{
	//	qDebug()<<__FILE__<<__FUNCTION__<<m_ref;
//	if (m_device && m_linked && m_doc && m_ref != 0) {
//		setFpgaAttrByAttr(m_doc, "M", 1);
//		setFpgaAttrByAttr(m_doc, "C", 0);
//		setFpgaAttrByAttr(m_doc, "D", 0);
//		if (m_ref == 1) {
//			setFpgaAttrByAttr(m_doc, "C", 0);
//			setFpgaAttrByAttr(m_doc, "D", 0);
//			setFpgaAttrByAttr(m_doc, "B", 1);
//		}
//		updateCtrlTable(true);
//	} else {
//		qDebug()<<__FILE__<<__FUNCTION__<<"device cann't control";
//	}
}

void DeviceUltrasound::running()
{
//	if (m_linked == true) {
//		static int ref = -1;
//		if (ref < 0) ref = nowRef();

//		if (ref == 1) {
//			ref = -1;
//		}else {
//			ref--;
//			return;
//		}
//		setFpgaAttrByAttr(m_doc, "FREEZE", 0);
//		m_device->running();
//		_updateCtrlTable();
//	} else {
//		qWarning() << "Err: " << __FILE__ << __LINE__ << "running";
//		QTimer::singleShot(100, this, SLOT(freezeSlot()));
//	}
}

void DeviceUltrasound::freeze()
{
//	if (m_linked == true) {
//		static int ref = -1;
//		if (ref < 0) ref = nowRef();

//		if (ref == 0) {
//			ref = -1;
//		}else {
//			ref--;
//			return;
//		}
//		setFpgaAttrByAttr(m_doc, "FREEZE", 1 );
//		_updateCtrlTable();
//		m_device->freeze();
//	} else {
//		qWarning() << "Err: " << __FILE__ << __LINE__ << "freeze";
//		QTimer::singleShot(100, this, SLOT(freezeSlot()));
//	}
}


int DeviceUltrasound::updateCtrlTable(bool flag){
    Q_UNUSED (flag);
//	static int ref = -1;
//	int ret = -1;
//	if (flag == true){
//		if (ref < 0) ref = nowRef();

//		if (ref == 1 ) {
//			ref = -1;
//		}else {
//			ref--;
//			return 0;
//		}

//		Args::instance()->updateArgs();
//		ret = _updateCtrlTable();

//		m_device->resetBuffer();

//	}else {
//		Args::instance()->updateArgs();
//		ret = _updateCtrlTable();

//		m_device->resetBuffer();
//	}

//	return ret;

    return 0;
}
//FIXME: realloc ??
int DeviceUltrasound::_updateCtrlTable() {
//	/*真正发送 所需的DBUS操作*/
//	int     bufSize = 0;
//	unsigned char *pTable = 0;

//	if (m_device) {
//		QDomNode node = m_doc->documentElement().firstChild();
//		while (!node.isNull()) {
//			if (node.isElement()) {
//				QDomElement e = node.toElement();
//				unsigned char argValue;
//				argValue = e.attribute("value", QString("0")).toInt();

//				int tmpByteOffset = e.attribute("byteoffset").toInt();
//				int tmpBitOffset = e.attribute("bitoffset").toInt();
//				int tmpBitLen = e.attribute("bitlen").toInt();

//				//                qDebug() << e.toElement().attribute("name") << tmpByteOffset << tmpBitOffset << tmpBitLen << e.attribute("value", QString("0")).toInt();
//				if (bufSize < tmpByteOffset + 1) {
//					bufSize = tmpByteOffset + 1;
//					pTable = (unsigned char *)realloc(pTable, sizeof(unsigned char)*(bufSize));
//					*(pTable + bufSize  - 1) = 0;
//				}

//				if (!pTable) {
//					qWarning() << "!!! Error:" << __FILE__ << __LINE__ << "realloc " << bufSize << "size";
//					exit(-1);
//				}

//				unsigned char *tmpValue = pTable + tmpByteOffset;
//				unsigned char tmp = 0;

//				for (int i = 0; i < tmpBitLen; ++i)
//					tmp |= 1 << (tmpBitOffset + i);
//				tmp = ~tmp;
//				*tmpValue &= tmp;

//				*tmpValue |= (argValue << tmpBitOffset);
//			}

//			node = node.nextSibling();
//		}
//		*pTable = 0;
//		if (!pTable)  {
//			qWarning() << "!!! Error:" << __FILE__ << __LINE__ << "pTable err";
//			return -1;
//		}



//		QByteArray ctrltable; int i = 0;
//		for (i = 0; i < bufSize; i++) {
//			ctrltable.append(pTable[i]);
//			ctrltable.append(i);
//		}

//		m_device->writeToFPGA(ctrltable);
//		if (pTable) free(pTable);

//	}

	return 0;
}

void  DeviceUltrasound::setFpgaXml(QDomDocument *d)
{
	m_doc = d;
}

QList <int>  DeviceUltrasound::updateProbes(){
	QList <int> list;

//	if (m_device && m_linked) {
//		QByteArray  tmp;
//		tmp = m_device->checkProbe(m_socket_max);
//		for (int i = 0; i < tmp.size(); i++) {
//			list << tmp.at(i);
//		}
//	}

	return list;
}

int DeviceUltrasound::nowRef()
{
	return m_ref;
}

MemoryUltrasound::MemoryUltrasound(QDomDocument *doc):Ultrasound("memory"){

	m_doc = doc->cloneNode().toDocument();
}

MemoryUltrasound::~MemoryUltrasound()
{

}

int MemoryUltrasound::updateCtrlTable(bool) { return -1; }

QList<int>   MemoryUltrasound::updateProbesInfo(){
	QList<int> list;
	return list;
}
#endif
