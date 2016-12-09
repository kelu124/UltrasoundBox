#ifndef CParams_H
#define CParams_H
#include <QObject>
#include <QDomDocument>
#include <QScriptEngine>

class CParamsPrivate;
class CParams : public QObject
{
    Q_OBJECT
public:
    CParams(QScriptEngine *engine, QDomNode &node);
    ~CParams();

public:
    Q_INVOKABLE void upParams() {
        emit paramsChanged();
    }

Q_SIGNALS:
    void paramsChanged();
private:
    CParamsPrivate *m_dptr;
};

#endif // CParams_H
