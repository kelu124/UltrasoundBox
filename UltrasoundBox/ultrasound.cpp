#include "ultrasound.h"

/*
    <root>
        <type>
    </root>

*/
Ultrasound * Ultrasound::newDevice (QDomDocument &machine \
                                   ,QDomDocument &fpga    \
                                   ,QDomDocument &soft    \
                                   ,QScriptEngine *engine)
{
    QDomElement root = machine.documentElement ();

    QDomElement typeNode = root.firstChildElement (QString("platform"));

    QString typeString = typeNode.attribute ("type", "64");

    if (typeString == QString("64")) {
        return new UltrasoundDevice64(machine, fpga, soft, engine);
    }

    return new UltrasoundDevice64(machine, fpga, soft, engine);
}

/*device*/
UltrasoundDevice64::UltrasoundDevice64(QDomDocument &machine, \
                                   QDomDocument &fpga,    \
                                   QDomDocument &soft, \
                                   QScriptEngine *engine):Ultrasound(fpga, soft, engine)
{
    Q_UNUSED (machine);
}

UltrasoundDevice64::~UltrasoundDevice64 ()
{

}

bool UltrasoundDevice64::init ()
{
    bool ret = false;

    return ret;
}

void UltrasoundDevice64::release ()
{

}

int UltrasoundDevice64::updateCtrlTable (bool)
{
    return 0;
}

int UltrasoundDevice64::updateSoftTable (bool)
{
    return 0;
}

/*memory*/
UltrasoundMemory64::UltrasoundMemory64(QDomDocument &machine, \
                                   QDomDocument &fpga,    \
                                   QDomDocument &soft,    \
                                   QScriptEngine *engine):Ultrasound(fpga, soft, engine)
{
    Q_UNUSED (machine);
}

UltrasoundMemory64::~UltrasoundMemory64()
{

}

bool UltrasoundMemory64::init ()
{
    bool ret = false;

    return ret;
}

void UltrasoundMemory64::release ()
{

}
