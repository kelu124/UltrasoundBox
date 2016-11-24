#ifndef ULTRASOUND_H
#define ULTRASOUND_H

#include <QObject>
#include <QDomDocument>
#include <QScriptEngine>
#include <QThread>
#if 0
#include "control_object.h"
#include "queue.h"

class Ultrasound:public QObject{

    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
public:
    enum DeviceType {
        DEVICE, MEMORY
    };
    Ultrasound( \
                QDomDocument &fpga,    \
                QDomDocument &soft,    \
                QScriptEngine *engine){

        m_fpga = fpga.cloneNode ().toDocument ();
        m_soft = soft.cloneNode ().toDocument ();

        m_engine  = engine;

        m_fpga_control = new Control(&m_fpga);
        m_soft_control = new Control(&m_soft);

        m_fpga_control->engineLinked (engine);
        m_soft_control->engineLinked (engine);
    }

    virtual ~Ultrasound() {
        if (m_fpga_control) delete m_fpga_control;
        if (m_soft_control) delete m_soft_control;
    }

    virtual  int          updateCtrlTable(bool) { return -1; }
    virtual  int          updateSoftTable(bool) { return -1; }

    virtual  QList <int>  updateProbesInfo(){ QList <int> list; return list;}

    virtual  bool         init ()   { return false;}
    virtual  void         release() {}

    static   Ultrasound   *newDevice(QDomDocument &machine, QDomDocument &fpga, QDomDocument &soft, QScriptEngine *engine);
protected:

    Control         *m_fpga_control;
    Control         *m_soft_control;

    QDomDocument     m_fpga;
    QDomDocument     m_soft;

    QScriptEngine  *m_engine;
    DeviceType      m_type;

};

class UltrasoundDevice64_Device_Usb:public QThread
{
    Q_OBJECT
    enum STATE{
        IDLE,
        PROBE,
        SEND,
        RECV,
    };
public:
    UltrasoundDevice64_Device_Usb() {
        exit = false;
        m_state = IDLE;
    }

    void run() Q_DECL_OVERRIDE {
        while(!exit) {
            switch(m_state) {
            case IDLE:
                break;
            case PROBE:
                break;
            case SEND:
                break;
            case RECV:
                break;
            }
        }
    }

signals:
    void probeReady(QList <int>);
private:
    void idle() {

    }
    void probe(){
        QList <int> probeList;
        for (int i = 0; i < m_max_sock; i++) {
            unsigned char buf[] = {0, 61};
            int size = sizeof(buf);
            send(buf, &size, 0);

            unsigned char query[] = {static_cast<unsigned char>((i<<4)), 0x14};
            size = sizeof(query);
            send(query, &size, 0);

            unsigned char recvbuf = -1;
            int           reclen  = 1;
            QThread::msleep(50);
            recvVer(&recvbuf, &reclen, 0);

            probeList << static_cast<int>(recvbuf);
        }
        emit probeReady(probeList);
    }
    void send( unsigned char *buf, int *size, int timeout);
    void recv( unsigned char *buf, int *size, int timeout);
    void recvVer( unsigned char *buf, int *size, int timeout);
    volatile bool exit;

    QList<int>    m_probe_list;
    int           m_max_sock;
    STATE         m_state;

};

class UltrasoundDevice64 :public Ultrasound
{
    Q_OBJECT
public:
    UltrasoundDevice64(QDomDocument &machine, \
            QDomDocument &fpga,    \
            QDomDocument &soft, QScriptEngine *engine);

    ~UltrasoundDevice64();

    bool  init();
    void  release();

    int   updateCtrlTable(bool);
    int   updateSoftTable(bool);

private:

    UltrasoundDevice64_Device_Usb   m_device_usb;

    CArrayQueue <_SingleFrame64>  m_buffer_b;
    CArrayQueue <_SingleFrame64>  m_buffer_c;
    CArrayQueue <_SingleLine64>   m_buffer_m;
    CArrayQueue <_ComplexIQ64>    m_buffer_d;

};

class UltrasoundMemory64: public Ultrasound
{
    Q_OBJECT
public:
    UltrasoundMemory64( QDomDocument &machine, \
            QDomDocument &fpga,    \
            QDomDocument &soft, QScriptEngine *engine);

    ~UltrasoundMemory64();

    bool  init();
    void  release();

};
#endif
#endif // ULTRASOUND_H
