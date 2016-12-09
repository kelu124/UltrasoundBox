#ifndef BParams_H
#define BParams_H
#include <QObject>
#include <QDomDocument>
#include <QScriptEngine>

class BParamsPrivate;
class BParams : public QObject
{
    Q_OBJECT
public:
    BParams(QScriptEngine *engine, QDomNode &node);
    ~BParams();

public:
    Q_INVOKABLE void upParams();
private:
    BParamsPrivate *m_dptr;
};

#endif // BParams_H
