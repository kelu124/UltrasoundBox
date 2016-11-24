#ifndef CONTROLOBJECT_H
#define CONTROLOBJECT_H

#include <QObject>
#include <QDomNode>
#include <QString>
#include <QScriptEngine>
<<<<<<< HEAD
#if 0
=======

>>>>>>> master
class ControlObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int     value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QString name  READ name)
    Q_PROPERTY(int     reset READ reset WRITE setReset NOTIFY resetChanged)

public:
    explicit ControlObject(QDomNode node, int value, int reset = 1, QObject *parent = 0);

    int value() const;
    void setValue(int);

    QString name() const;

    int reset() const;
    void setReset(int);

    void updateNode(QDomNode);
protected:
    QDomNode    m_node;
signals:
    void valueChanged();
    void resetChanged();

public slots:
};

class Control: public QObject
{
    Q_OBJECT
public:
    /*file*/
    Control (QString );
    Control (QDomDocument *doc);
    ~Control();

    void engineLinked(QScriptEngine *);
    void engineUnLinked();

    void updateData(QDomDocument *);

    void setObjectValue(QString, int);
    int  objectValue(QString);

private:
    QHash <QString, ControlObject* >    m_hash_controls;
    QScriptEngine                      *m_engine;

    QDomDocument                       *m_doc;
};
<<<<<<< HEAD
#endif
=======
>>>>>>> master
#endif // CONTROLOBJECT_H
