#ifndef DParams_H
#define DParams_H
#include <QObject>
#include <QDomDocument>
#include <QScriptEngine>

class DParamsPrivate;
class DParams : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int audio READ audio WRITE setAudio NOTIFY audioChanged)
    Q_PROPERTY(int width READ width  WRITE setWidth  NOTIFY widthChanged)
    Q_PROPERTY(int height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(float linepersec READ linepersec WRITE setLinepersec NOTIFY linepersecChanged)
    Q_PROPERTY(int prf READ prf WRITE setPrf NOTIFY prfChanged)
    Q_PROPERTY(int ver READ ver WRITE setVer NOTIFY verChanged)
    Q_PROPERTY(int baseline READ baseline WRITE setBaseline NOTIFY baselineChanged)
    Q_PROPERTY(int autotrace READ autotrace WRITE setAutotrace NOTIFY autotraceChanged)
    Q_PROPERTY(int invert READ invert WRITE setInvert NOTIFY invertChanged)
    Q_PROPERTY(int optid READ optid WRITE setOptid NOTIFY optidChanged)
    Q_PROPERTY(int resolution READ resolution WRITE setResolution NOTIFY resolutionChanged)
	Q_PROPERTY(int triplex READ triplex WRITE setTriplex NOTIFY triplexChanged)

	Q_PROPERTY(int maxReplay READ maxReplay WRITE setMaxReplay NOTIFY maxReplayChanged)
    Q_PROPERTY(int currentMax READ currentMax WRITE setCurrentMax NOTIFY currentMaxChanged)
public:
    DParams(QScriptEngine *engine, QDomNode &node);
    ~DParams();
public:
    int audio () const;
    void setAudio (int _value);
    int width () const;
    void setWidth (int _value);
    int height () const;
    void setHeight (int _value);
    float linepersec () const;
    void setLinepersec (float _value);
    int prf () const;
    void setPrf (int _value);
    int ver () const;
    void setVer (int _value);
    int baseline () const;
    void setBaseline (int _value);
    int autotrace () const;
    void setAutotrace (int _value);
    int invert () const;
    void setInvert (int _value);
    int optid () const;
    void setOptid (int _value);
    int resolution () const;
    void setResolution (int _value);
    int triplex () const;
    void setTriplex (int _value);

	int maxReplay() const;
	void setMaxReplay(int);
    int currentMax() const;
    void setCurrentMax(int);
signals:
    void audioChanged();
    void widthChanged();
    void heightChanged();
    void linepersecChanged();
    void prfChanged();
    void verChanged();
    void baselineChanged();
    void autotraceChanged();
    void invertChanged();
    void optidChanged();
    void resolutionChanged();
    void triplexChanged();

	void maxReplayChanged();
    void currentMaxChanged();
    void paramsChanged();
public:
    Q_INVOKABLE void upParams() {
        emit paramsChanged();
    }
    int getVBoxID();
    int SetDParams(int pos, int value);
    int SetDParams(int pos, float value);
private:
    DParamsPrivate *m_dptr;
};

#endif // DParams_H
