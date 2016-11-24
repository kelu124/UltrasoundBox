#ifndef ULTRASOUND_H
#define ULTRASOUND_H

#include <QHash>
#include <QObject>
#include <QString>
#include <QDomDocument>
#include <QFile>
#include <QDir>
#include <QBuffer>
#include <QTimer>
#include <QMutex>
#include <QByteArray>
#include <QScriptEngine>

#include <QDebug>
#if 0
class Ultrasound;

class FpgaControlObject:public QObject
{
	Q_OBJECT
	Q_PROPERTY(int     value READ value WRITE setValue NOTIFY valueChanged)
	Q_PROPERTY(QString name  READ name)
    Q_PROPERTY(int     reset READ reset)
public:
    FpgaControlObject(QDomNode &node, int value, int reset);
	~FpgaControlObject();

	int value() const;
	void setValue(int value);

    int reset() const;

	QString name() const;

Q_SIGNALS:
		void valueChanged();
        void resetChanged();
private:
	QDomNode    m_node;
};
class SoftControlObject:public QObject
{
	Q_OBJECT
	Q_PROPERTY(int     value READ value WRITE setValue NOTIFY valueChanged)
	Q_PROPERTY(QString name  READ name)
public:
	SoftControlObject(QDomNode &node, int value);
	~SoftControlObject();

	int value() const;
	void setValue(int value);

	QString name() const;

Q_SIGNALS:
	void valueChanged();

private:
	QDomNode    m_node;
};

class SoftControl:public QObject
{
	Q_OBJECT
public:
    SoftControl(QString xmlFileName);
	SoftControl(QDomDocument *doc);
	~SoftControl();

	void engineLinked(QScriptEngine *);
	void changedSource(QDomDocument *doc);
	void changedUltrasound(Ultrasound *u);

    void setObjectValueFName(QString name, QString value);
    int objectValueFName(QString name);

public Q_SLOTS:
    void objectValueChanged();

private:
	QDomDocument     *m_doc;
    QHash <QString, SoftControlObject* >    m_hash_controls;
    Ultrasound                              *m_ultrasound;
    QScriptEngine                           *m_engine;
};
class FpgaControl:public QObject
{
	Q_OBJECT
public:
	FpgaControl(QString xmlFileName);
	FpgaControl(QDomDocument *doc);
	~FpgaControl();

    void engineLinked(QScriptEngine *);
	void changedSource(QDomDocument *doc);
	void changedUltrasound(Ultrasound *u);

	void setObjectValueFName(QString name, QString value);
    int  objectValueFName(QString name);

public Q_SLOTS:
	void objectValueChanged();

    void resetChanged();
private:
	QDomDocument                           *m_doc;
	QHash <QString, FpgaControlObject* >    m_hash_controls;

	Ultrasound                             *m_ultrasound;
	QScriptEngine                          *m_engine;

    bool                                    m_reset_flag;
};

class Ultrasound:public QObject{

	Q_OBJECT
	Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
public:
    enum DeviceType {
        DEVICE, MEMORY
    };
    Ultrasound( QDomDocument &machine, \
                QDomDocument &fpga,    \
                QDomDocument &soft): m_machine(machine), \
                                     m_fpga(fpga), \
                                     m_soft(soft){

	}

	virtual ~Ultrasound() {

	}

	virtual  int          updateCtrlTable(bool) { return -1; }
    virtual  int          updateSoftTable(bool) { return -1; }

protected:

    QDomDocument     m_machine;
    QDomDocument     m_fpga;
    QDomDocument     m_soft;

    DeviceType      m_type;
};


class DeviceUltrasound:public Ultrasound
{
	Q_OBJECT
public:
    DeviceUltrasound( QDomDocument &machine, \
                      QDomDocument &fpga,    \
                      QDomDocument &soft);

	int    updateCtrlTable(bool);
    int    updateSoftTable(bool);

	QList <int>  updateProbesInfo(){ return updateProbes();}
	QList <int>  updateProbes();

	int   addRef();
	int   decRef();
	int   nowRef();

	void  running();
	void  freeze();

	void  setFpgaXml(QDomDocument *);
	QDomDocument  *document();
private Q_SLOTS:
	void    checkDevice() {
//        if (m_device->init()){
//            qDebug() << "Device Init ...... OK";
//            m_linked = true;
//        }else{
//			QTimer::singleShot(1000, this, SLOT(checkDevice()));
//			m_linked = false;
//            qDebug() << __FILE__ << __LINE__ << "Device Init Error ....... reseting";
//        }
	}

	void  runningSlot() {
		if (m_linked == true) {
            qDebug() << "Device Running ...... OK";
		}else {
            qDebug() << "Error: " << __FILE__ << __LINE__ << "Device running Error ........ reseting";
			QTimer::singleShot(100, this, SLOT(runningSlot()));
		}
	}

	void freezeSlot() {
		if (m_linked == true) {
            qDebug() << "Device Freeze ...... OK";
		}else {
            qDebug() << "Error: " << __FILE__ << __LINE__ << "Device Freeze Error ........ reseting";
			QTimer::singleShot(100, this, SLOT(freezeSlot()));
		}
	}

private:

	int    _updateCtrlTable();

	bool                m_linked;

	QDomDocument        *m_doc;
	volatile int         m_ref;

};

class MemoryUltrasound:public Ultrasound
{
	Q_OBJECT
public:
    MemoryUltrasound(QDomDocument &machine, QDomDocument &fpga, QDomDocument &soft);
	virtual ~MemoryUltrasound();

	int    updateCtrlTable(bool);
    int    updateSoftTable(bool);

	QList<int>   updateProbesInfo();

    void
private:

};
#endif
#endif
