#include "dllmanager.h"
#include <QDir>
#include <QCoreApplication>

#include "ZJIF_Doppler.h"
#include "imageOpt.h"
#include <QVariant>
#include <QDebug>

#define MODULE_NAME "DllManager"
static  const QString ImageOptName = "ImageOpt";
static  const QString DopplerName  = "SCDoppler";

DllManager::DllManager(QObject *parent): QObject(parent)
{
    setObjectName(MODULE_NAME);
}
void DllManager::init()
{
    currentOptIndex = 0;
    currentDopIndex = 0;
    unloadLibDoppler();
    unloadLibOpt();

    loadLibOpt();
    loadLibDoppler();
    qRegisterMetaType<DllManager *>(MODULE_NAME);
    qApp->setProperty(MODULE_NAME, QVariant::fromValue(DllManager::instance()));
}
void DllManager::release()
{
    unloadLibDoppler();
    unloadLibOpt();
}
ZJIF_ImageOpt * DllManager::getImageOpt()
{
    currentOptIndex = (currentOptIndex + 1 ) % opts.length();
    qDebug() << "Dllmanager use ImageOpt " << currentOptIndex;
    return opts[currentOptIndex].second;
}
ZJIF_Doppler  * DllManager::getDoppler()
{
    currentDopIndex = (currentDopIndex + 1) % dops.length();
    qDebug() << "Dllmanager use Doppler " << currentDopIndex;
    return dops[currentDopIndex].second;
}

void DllManager::loadLibOpt()
{
    QDir dir(QDir::currentPath() + QDir::separator() + "dll");
    char buf[] = "hello";
    typedef void* (*lpImgOptFun)(char *pGUID);
    lpImgOptFun pImgOptFun = NULL;
    ZJIF_ImageOpt *pImgopt = NULL;
    for (QString fileName : dir.entryList(QDir::Files)) {
        if (fileName.contains(ImageOptName) && QLibrary::isLibrary(fileName)) {
            QLibrary *lib = new QLibrary;
            lib->setFileName(dir.absolutePath() + dir.separator() + fileName);
            if (lib->load()) {
                pImgOptFun  = NULL;
                pImgOptFun  = (lpImgOptFun)lib->resolve("GetInstance");
                if (pImgOptFun) {
                    pImgopt  = NULL;
                    pImgopt  = (ZJIF_ImageOpt *)pImgOptFun(buf);
                    if (pImgopt) {
                        QPair<QLibrary *, ZJIF_ImageOpt *> pair;
                        pair.first  = lib;
                        pair.second = pImgopt;
                        opts.append(pair);
                        qDebug() << lib->fileName() << "load OK"<< pImgopt;
                    } else {
                        qDebug()<< "call GetInstance function failed";
                        delete lib;
                        continue;
                    }
                } else {
                    qDebug() << "get GetInstance function failed";
                    delete lib;
                    continue;
                }
            } else {
                qDebug()<<__FILE__<<__LINE__<<"load failed!"<< lib->fileName() <<lib->errorString();
                qDebug() << QDir::currentPath() << qApp->applicationDirPath();
                delete lib;
            }
        }
    }
}

void DllManager::loadLibDoppler()
{
    QDir dir(QDir::currentPath() + QDir::separator() + "dll");
    typedef void*(*lpPW)(void);
    lpPW pPW = NULL;
    ZJIF_Doppler *pDoppler= NULL;

    for (QString fileName : dir.entryList(QDir::Files)) {
        if (fileName.contains(DopplerName) && QLibrary::isLibrary(fileName)) {
            QLibrary *lib = new QLibrary;
            lib->setFileName(dir.absolutePath() + dir.separator() + fileName);
            if (lib->load()) {
                pPW  = NULL;
                pPW  = (lpPW)lib->resolve("GetInstance");
                if (pPW) {
                    pDoppler = NULL;
                    pDoppler = (ZJIF_Doppler *)pPW();
                    if (pDoppler) {
                        QPair<QLibrary *, ZJIF_Doppler*> pair;
                        pair.first   = lib;
                        pair.second  = pDoppler;
                        dops.append(pair);
                        qDebug() << lib->fileName() << "load OK" << pDoppler;
                    } else {
                        qDebug()<< "call GetInstance function failed";
                        delete lib;
                        continue;
                    }
                } else {
                    qDebug() << "get GetInstance function failed";
                    delete lib;
                    continue;
                }
            } else {
                qDebug()<<__FILE__<<__LINE__<<"load failed!"<<lib->fileName()<<lib->errorString();
                qDebug() << QDir::currentPath() << qApp->applicationDirPath();
                delete lib;
            }
        }
    }
}

void DllManager::unloadLibOpt()
{
    for (auto i : opts) {
        if (i.second) {
            i.second->End();
            delete i.second;
            i.second = NULL;
        }
        if (i.first ) {
            if (i.first->isLoaded()) {
                i.first->unload();
            }
            delete i.first;
            i.first = NULL;
        }
    }
    opts.clear();
}

void DllManager::unloadLibDoppler()
{
    for (auto i : dops) {
        if (i.second) {
            i.second->DEnd();
            delete i.second;
            i.second = NULL;
        }
        if (i.first ) {
            if (i.first->isLoaded()) {
                i.first->unload();
            }
            delete i.first;
            i.first = NULL;
        }
    }
    dops.clear();
}
