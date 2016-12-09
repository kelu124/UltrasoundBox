#ifndef DLLMANAGER_H
#define DLLMANAGER_H

#include <QLibrary>

class ZJIF_ImageOpt;
class ZJIF_Doppler;

class DllManagerAbstract{
public:
    virtual ~DllManagerAbstract(){}
    virtual void init()     = 0;
    virtual void release()  = 0;
    virtual ZJIF_ImageOpt * getImageOpt() = 0;
    virtual ZJIF_Doppler  * getDoppler()  = 0;
};

class DllManager : public QObject, public DllManagerAbstract
{
    Q_OBJECT
public:

    static DllManager * instance()
    {
        static  DllManager * _dll = new DllManager;
        return  _dll;
    }
    void init()     override;
    void release()  override;
    ZJIF_ImageOpt * getImageOpt() override;
    ZJIF_Doppler  * getDoppler()  override;

private:
    DllManager(QObject *parent = 0);
    void loadLibOpt();
    void unloadLibOpt();

    void loadLibDoppler();
    void unloadLibDoppler();

    QList<QPair<QLibrary *, ZJIF_ImageOpt *>> opts;
    QList<QPair<QLibrary *, ZJIF_Doppler *>> dops;
    int     currentOptIndex;
    int     currentDopIndex;
};

#endif // DLLMANAGER_H
