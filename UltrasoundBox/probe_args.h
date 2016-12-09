#ifndef PROBE_INFO_H
#define PROBE_INFO_H

#include <QObject>
#include <QDomDocument>
#include <QScriptEngine>

class ProbeArgs:  public QObject
{
    Q_OBJECT
    Q_PROPERTY(int   element   READ element   WRITE setElement   NOTIFY elementChanged)
    Q_PROPERTY(float interval  READ interval  WRITE setInterval  NOTIFY intervalChanged)
    Q_PROPERTY(float radius    READ radius    WRITE setRadius    NOTIFY radiusChanged)
    Q_PROPERTY(double angle    READ angle     WRITE setAngle     NOTIFY angleChanged)
    Q_PROPERTY(float lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    Q_PROPERTY(int   type      READ type      WRITE setType      NOTIFY typeChanged)
    Q_PROPERTY(float wobbling  READ wobbling  WRITE setWobbling  NOTIFY wobblingChanged)
    Q_PROPERTY(int   slice1    READ slice1    WRITE setSlice1    NOTIFY slice1Changed)
    Q_PROPERTY(int   slice2    READ slice2    WRITE setSlice2	 NOTIFY slice2Changed)
    Q_PROPERTY(int   slice3    READ slice3    WRITE setSlice3    NOTIFY slice3Changed)
    Q_PROPERTY(int   slice4    READ slice4    WRITE setSlice4    NOTIFY slice4Changed)

public:
    ProbeArgs(QDomDocument *doc, QScriptEngine *);
    ~ProbeArgs();

    int element() const;
    void setElement(int);

    float interval() const;
    void  setInterval(float);

    float radius() const;
    void  setRadius(float);

    double angle() const;
    void  setAngle(double);

    float lineWidth() const;
    void  setLineWidth(float);

    int	  type() const;
    void  setType(int);

    float wobbling() const;
    void  setWobbling(float);

    int	  slice1() const;
    void  setSlice1(int);

    int	  slice2() const;
    void  setSlice2(int);

    int	  slice3() const;
    void  setSlice3(int);

    int	  slice4() const;
    void  setSlice4(int);
Q_SIGNALS:
    void elementChanged();
    void intervalChanged();
    void radiusChanged();
    void angleChanged();
    void lineWidthChanged();
    void typeChanged();

    void wobblingChanged();
    void slice1Changed();
    void slice2Changed();
    void slice3Changed();
    void slice4Changed();
private:
    QScriptEngine *m_engine;
    QDomDocument  *m_doc;
    QDomElement	   m_element, m_interval, m_radius, m_angle, m_lineWidth, m_type, m_wobbling, m_slice1, m_slice2, m_slice3, m_slice4;
};
#endif //
