#ifndef RENDERBASE_H
#define RENDERBASE_H

#include <QOpenGLContext>
#include <QOpenGLTexture>
#include <QOpenGLShader>
#include <QDomDocument>
#include <QOpenGLFunctions>

class RenderBase : protected QOpenGLFunctions
{
public:
    RenderBase() {}

    virtual ~RenderBase() {}
    virtual void initialize() {}
    virtual void paint() {}
    virtual void createGeometry() {}
    virtual void setMap   ( int, unsigned char *, int ) {}
    virtual void addData  ( int, char *, int ) {}
    virtual void setScan(QDomNode) {}

    void setViewPort (int vx, int vy, int vw, int vh);
    void setViewPort (QRect rect);

    void setGeoTrans (QDomNode);
    void setGeoProbe (QDomNode);

    QByteArray readFile(QString &);
    void loadResure(QDomNode);

    QMatrix4x4 mModelMatrix;
    QMatrix4x4 mViewMatrix;
    QMatrix4x4 mProjectionMatrix;

    QMatrix4x4 mMVPMatrix;
    QSize      mSize;

    QVector<QVector3D> colorsMapB;
    QVector<QVector3D> colorsMapC;
    QVector<QVector3D> colorsMapM;
    QVector<QVector3D> colorsMapPW;

    QVector<QVector3D> mVerticesB;
    QVector<QVector2D> mTexcoordsB;

    QVector<QVector3D> mVerticesC;
    QVector<QVector2D> mTexcoordsc;

    QVector<QVector3D> mVerticesM;
    QVector<QVector2D> mTexcoordsM;

    QVector<QVector3D> mVerticesPW;
    QVector<QVector2D> mTexcoordsPW;


    struct mapData {
        unsigned char b_colormaplist[8][256][3];
        unsigned char c_colormaplist[8][256][3];
    };

    struct st_img_trans{
        int            grayReverse;
        struct         mapData colormap;
    }m_img_trans;


    struct st_geo_trans{
        int        angle;
        int        x_turn;
        int        y_turn;
        QRectF     cutRect;
    }m_geo_trans;


    struct st_Probe {
        int   hard;
        int   soft;

        int   element;
        float interval;
        float radius;
        float angle;
        float linewidth;

        QString name;
        QString icon;
        QString type;

    }m_probe;

    QMutex      m_mutex;
};


#endif // RENDERBASE_H
