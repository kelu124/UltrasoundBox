#ifndef __VIRTUAL_BOX_H__
#define __VIRTUAL_BOX_H__

#include <QObject>
#include <QPoint>

#include <QScriptEngine>
#include <QScriptEngineDebugger>
#include <QDomDocument>
#include <QQuickItem>
#include <QQuickPaintedItem>
#include <QThread>
#include <QList>

#include <QRect>
#include <QObject>
#include <QString>

#include <QTimer>
#include <QDomDocument>

#include <QDir>
#include "render_item.h"
#include "control_object.h"
#include "IO/io_controls.h"

#include "layout_params.h"
#include "b_params.h"
#include "c_params.h"
#include "m_params.h"
#include "d_params.h"

#include "probe_args.h"
#include "plugins.h"
#include "echo_plugins_manager.h"


#include <QList>

class VirtualBox: public QObject
{
    Q_OBJECT

public:
    VirtualBox( int id
              , IOControls   *io
              , QDomDocument &probe
              , QDomDocument &device
              , QDomDocument &soft
              , QQuickItem *gui) {

        QDomNode renderNode;

        /*ID*/
        m_id = id;

        /*HMI*/
        m_hmi.io = io;

        /*engine init*/
        m_engine = new QScriptEngine;
        jsLoaderFirst ();

        QDomNode layout;
        setLayout (layout);

        /*Objects Register start*/
        /*probe args init*/
        m_probe  = new ProbeArgs(&probe, m_engine);

        /*fpga soft objects init*/
        m_fpga_control = new Control(&device);
        m_soft_control = new Control(&soft);

        /*fpga soft links*/
        m_fpga_control->engineLinked(m_engine);
        m_soft_control->engineLinked(m_engine);

        /*mode params*/
        QDomNode modeNode;
        m_mode_params.b_params = new BParams(m_engine, modeNode);
        m_mode_params.c_params = new CParams(m_engine, modeNode);
        m_mode_params.d_params = new DParams(m_engine, modeNode);
        m_mode_params.m_params = new MParams(m_engine, modeNode);
        /*Objects Register end*/

        /*JSLoader*/
        jsLoaderSecond();

        /*Plugins*/
        functionLoader();
        /*mode*/

        /*创建绘制线程*/
        m_render= new RenderItem(renderNode, gui);

    }

    ~VirtualBox()
    {

    }


private:

    void layoutSetting(){

    }

    void functionLoader()
    {
//        /*load eplugins*/
//        int mode = -1;
//        if("B" == runMode)
//            mode = EchoPosition_B;
//        else if("C" == runMode)
//            mode = EchoPosition_C;
//        else if("D" == runMode)
//            mode = EchoPosition_D;
//        else if("M" == runMode)
//            mode = EchoPosition_M;

//        if(-1 == mode)
//            qDebug() << "Not Correpond...";

//        m_eplugins = PluginsManager::instance()->newItems(m_echo, m_engine, mode, m_vbox_id);
//        PluginsManager::instance()->linkItems(m_vbox_id);

//        ImagePluginsManager::instance()->setMode(m_vbox_id, mode);
//        ImagePluginsManager::instance()->newItems(m_echo, m_engine,DllManager::instance(), m_vbox_id);
//        ImagePluginsManager::instance()->links(m_vbox_id);
    }

    void jsLoaderSecond() {
        QFile mainFile(QDir::currentPath () + "/js/main.js");

        if (!mainFile.exists ()) {
            if (mainFile.open (QIODevice::ReadOnly | QIODevice::Text)){
                QByteArray content = mainFile.readAll ();
                m_engine->evaluate (content);
                if (m_engine->hasUncaughtException ()) {
                    qWarning() << "main.js" << m_engine->uncaughtExceptionLineNumber() \
                               << " "<< m_engine->uncaughtException ().toString ();
                    m_engine->clearExceptions ();
                }
            }

        }
        /* add third js folder*/
        QDir pluginsDir(QDir::currentPath() + "/js/thirds/");

        foreach(QString fileName, pluginsDir.entryList(QDir::Files)) {
            QFile file(QDir::currentPath() + "/js/third/" + fileName);
            if (file.exists ()) continue;
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

            QByteArray content =  file.readAll();
            m_engine->evaluate (content);

            if(m_engine->hasUncaughtException()) {
                qDebug() << "Error:" << "js/third/" + fileName << "jsLoader: evaluate() "<< m_engine->uncaughtExceptionLineNumber() \
                         << " "<< m_engine->uncaughtException ().toString ();
                m_engine->clearExceptions();
            }
        }
    }

    void jsLoaderFirst(){
        QFile mainFile(QDir::currentPath () + "/js/prevalue.js");

        if (!mainFile.exists ()) {
            if (mainFile.open (QIODevice::ReadOnly | QIODevice::Text)){
                QByteArray content = mainFile.readAll ();
                m_engine->evaluate (content);
                if (m_engine->hasUncaughtException ()) {
                    qWarning() << "prevalue.js" << m_engine->uncaughtExceptionLineNumber() \
                               << " "<< m_engine->uncaughtException ().toString ();
                    m_engine->clearExceptions ();
                }
            }

        }
    }

    void setLayout(QDomNode node) {
        m_setting.layout_params = new LayoutParams(m_engine, node);
        QScriptValue funcall = m_engine->globalObject ().property ("LayoutMain");
        if (!funcall.isValid ()){
            funcall.call (QScriptValue());
            if (m_engine->hasUncaughtException ()) {
                qWarning() << "LayoutMain" << "Err";
                return;
            }
        }
    }

    void setDeviceConfig(QDomNode node) {

    }

    QScriptEngine   *m_engine;

    ProbeArgs       *m_probe;
    Control         *m_fpga_control;
    Control         *m_soft_control;

    /*Render Thread*/
    RenderItem      *m_render;

    /*vbox ID */
    int              m_id;

    struct st_layout{
        LayoutParams    *layout_params;
    }m_setting;

    struct st_plugins{
        QList<VirtualScottEchoAssist *> m_eplugins;
    }m_plugins;

    struct st_mode_params{
        BParams     *b_params;
        CParams     *c_params;
        DParams     *d_params;
        MParams     *m_params;
    }m_mode_params;

    struct st_io {
        IOControls  *io;
    }m_hmi;

    struct st_transition{
        int transBIndex;
        int transCIndex;
        int transDIndex;
        int transMIndex;
    }m_trans_image;
};

#endif // VIRTUALBOX_H
