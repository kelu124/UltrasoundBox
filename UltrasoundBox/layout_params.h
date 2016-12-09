#ifndef LayoutParams_H
#define LayoutParams_H
#include <QObject>
#include <QDomDocument>
#include <QScriptEngine>

class LayoutParamsPrivate;
class LayoutParams : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int echoX  READ echoX  WRITE setEchoX  NOTIFY echoXChanged)
    Q_PROPERTY(int echoY  READ echoY  WRITE setEchoY  NOTIFY echoYChanged)
    Q_PROPERTY(int echoW  READ echoW  WRITE setEchoW  NOTIFY echoWChanged)
    Q_PROPERTY(int echoH  READ echoH  WRITE setEchoH  NOTIFY echoHChanged)

    Q_PROPERTY(int imageX READ imageX WRITE setImageX NOTIFY imageXChanged)
    Q_PROPERTY(int imageY READ imageY WRITE setImageY NOTIFY imageYChanged)
    Q_PROPERTY(int imageW READ imageW WRITE setImageW NOTIFY imageWChanged)
    Q_PROPERTY(int imageH READ imageH WRITE setImageH NOTIFY imageHChanged)

public:
    LayoutParams(QScriptEngine *engine, QDomNode &node);
    ~LayoutParams();

    int echoX() const;
    int echoY() const;
    int echoW() const;
    int echoH() const;

    int imageX() const;
    int imageY() const;
    int imageW() const;
    int imageH() const;

    void setEchoX(int);
    void setEchoY(int);
    void setEchoW(int);
    void setEchoH(int);

    void setImageX(int);
    void setImageY(int);
    void setImageW(int);
    void setImageH(int);

Q_SIGNALS:
    void echoXChanged();
    void echoYChanged();
    void echoWChanged();
    void echoHChanged();

    void imageXChanged();
    void imageYChanged();
    void imageWChanged();
    void imageHChanged();

private:
    LayoutParamsPrivate *m_dptr;
};

#endif // LayoutParams_H
