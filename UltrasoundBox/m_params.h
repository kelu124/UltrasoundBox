#ifndef MParams_H
#define MParams_H
#include <QObject>
#include <QDomDocument>
#include <QScriptEngine>

class MParamsPrivate;
class MParams : public QObject
{
    Q_OBJECT

public:
    MParams(QScriptEngine *engine, QDomNode &node);
    ~MParams();
public:
    Q_INVOKABLE void upParams() {
        emit paramsChanged();
    }

Q_SIGNALS:
    void paramsChanged();
private:
    MParamsPrivate *m_dptr;
};

#endif // MParams_H
