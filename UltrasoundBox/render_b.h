#ifndef RENDER_B_H
#define RENDER_B_H

#include "render_base.h"

struct st_opengl_data {
    QOpenGLTexture    *texture;
    QVector<QVector3D> vertices;
    QVector<QVector2D> vercoords;
};

struct st_complex{
    struct st_opengl_data opengl_data_l;
    struct st_opengl_data opengl_data_m;
    struct st_opengl_data opengl_data_r;

};

class RenderB : public RenderBase
{
public:
    RenderB(QSize);
/*remmap*/
    void initialize();
    void paint();
    void createGeometry();

    void setMap  ( int, unsigned char *, int );
    void addData (int, char *, int );
    void setScan (QDomNode);

private:
    void initShaders();
    void initData();

    void drawDSC();

    QOpenGLTexture *getTextures();

    QOpenGLShaderProgram	 program;

    int mMVPMatrixHandle;
    int mPositionHandle;
    int mTexCoordHandle;
    int mColorArrayhandle;

    struct st_scan{
        float depth;

        struct st_opengl_data mData;
    }m_scan;

    struct st_textureGeo{
        int     lines;
        int     samples;
        st_textureGeo() {
            lines = 256;
            samples = 512;
        }
    }m_texture_geo;

    QByteArray      mByteArrayData;
};

#endif // RENDER_B_H
