#include "virtual_box.h"

#include <QDebug>


#include <QString>

#include <QMutex>
#include <QThread>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <qsgsimpletexturenode.h>

#include <qopenglshaderprogram.h>
#include <qopenglfunctions.h>
#include <qopenglfunctions.h>
#include <QQuickWindow>
#include <QOpenGLTexture>
#include <QOpenGLShader>

#include <QDomNode>

#include <QDir>
#include <QQuickWindow>

#include "eplugins_manager.h"
#include <qmath.h>

class RenderBase : protected QOpenGLFunctions
{
public:
    RenderBase() {}

    virtual ~RenderBase() {}
    virtual void initialize() {}
    virtual void paint() {}
    virtual void createGeometry() {}
    virtual void setMap   ( int, unsigned char *, int ) {}
    virtual void addData  ( int, unsigned char *, int ) {}
    virtual void setScan(QDomNode) {}

    void setViewPort (int vx, int vy, int vw, int vh);
    void setViewPort (QRect rect);

    void setGeoTrans (QDomNode);
    void setGeoProbe (QDomNode);

    QMatrix4x4 mModelMatrix;
    QMatrix4x4 mViewMatrix;
    QMatrix4x4 mProjectionMatrix;

    QMatrix4x4 mMVPMatrix;
    QSize      mSize;

    QVector<QVector3D> colorsMapB;
    QVector<QVector3D> colorsMapC;
    QVector<QVector3D> colorsMapM;
    QVector<QVector3D> colorsMapPW;

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
};

void RenderBase::setViewPort (int vx, int vy, int vw, int vh)
{
    Q_UNUSED(vx);
    Q_UNUSED(vy);
    mViewMatrix.setToIdentity();
    mViewMatrix.lookAt(QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0, 0.0f, -5.0f), QVector3D(0.0f, 1.0f, 0.0f));
    mViewMatrix.scale(mSize.width ()/vw, mSize.height ()/vh, 1.0);
}

void RenderBase::setViewPort (QRect rect)
{
    setViewPort(rect.x(), rect.y(), rect.width(), rect.height());
}

void RenderBase::setGeoTrans (QDomNode root)
{
    if (root.isNull ()) return;

    qDebug() << root.toElement ().tagName ();

    QDomNode node = root.firstChildElement ();

    while(!node.isNull ()) {
        if (node.isElement ()) {
            if (node.toElement ().tagName () == QString("rotate")) {
                int angle = node.toElement ().attribute ("value", QString("0")).toInt ();
                if (angle == 0) {
                    mModelMatrix.setToIdentity ();
                }else {
                    mModelMatrix.rotate (angle, 0.0f, 0.0f, 1.0f);
                }
                m_geo_trans.angle = angle;
            }else if (node.toElement ().tagName () == QString("turnUD")){
                int v = node.toElement ().attribute ("value", QString("0")).toInt();
                if (v == 0) {
                    m_geo_trans.y_turn = 1;
                    mModelMatrix.setToIdentity ();
                }else {
                    m_geo_trans.y_turn = -1;
                    mModelMatrix.rotate (180, 1, 0, 0);
                }
            }else if (node.toElement ().tagName () == QString("turnLR")) {
                int v = node.toElement ().attribute ("value", QString("0")).toInt ();

                if (v == 0) {
                    m_geo_trans.x_turn = 1;
                    mModelMatrix.setToIdentity ();
                }else {
                    m_geo_trans.x_turn = -1;
                    mModelMatrix.rotate (180, 1, 0, 0);
                }
            }else if (node.toElement ().tagName () == QString("size")) {
                int w = node.toElement ().attribute ("width", QString("0")).toInt ();
                int h = node.toElement ().attribute ("height", QString("0")).toInt ();

                mSize = QSize(w, h);
            }
        }

        node = node.nextSiblingElement ();
    }

}

void RenderBase::setGeoProbe (QDomNode node)
{
    if (node.isNull ()) {
        return;
    }

    QString probeType = node.toElement ().attribute ("type");

    if (probeType == QString("Convex")) {
        m_probe.name = node.toElement ().attribute ("name", "");
        m_probe.icon = node.toElement ().attribute ("icon", "");
        m_probe.hard = node.toElement ().attribute ("hard", "0").toInt ();
        m_probe.soft = node.toElement ().attribute ("soft", "").toInt ();
        m_probe.type = probeType;
        m_probe.element = node.toElement ().attribute ("element").toInt();
        m_probe.interval= node.toElement ().attribute ("interval", "0").toFloat ();
        m_probe.radius  = node.toElement ().attribute ("radius", "0").toFloat ();
        m_probe.angle   = node.toElement ().attribute ("angle", "0").toFloat ();
    }else if (probeType == QString("Line")) {
        m_probe.name = node.toElement ().attribute ("name", "");
        m_probe.icon = node.toElement ().attribute ("icon", "");
        m_probe.hard = node.toElement ().attribute ("hard", "0").toInt ();
        m_probe.soft = node.toElement ().attribute ("soft", "").toInt ();
        m_probe.type = probeType;
        m_probe.element  = node.toElement ().attribute ("element").toInt();
        m_probe.interval = node.toElement ().attribute ("interval", "0.0").toFloat ();
        m_probe.linewidth= node.toElement ().attribute ("lineWidth").toFloat ();
    }else {
        qDebug() << "not support probe";
    }

}


struct st_opengl_data {
    int    flag;
    float  angle;
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
    void addData ( int, unsigned char *, int );
    void setScan (QDomNode);

private:
    void initShaders();
    void initData();

    void drawDSC();

    QOpenGLTexture *getTextures();

    QOpenGLShaderProgram	 program;

    int mModelMatrixHandle;
    int mViewMatrixHandle;
    int mProjectionMatrixHandle;

    int mPositionHandle;
    int mTexCoordHandle;
    int mColorArrayhandle;


    struct st_scan{
        float depth;

        int    complex_len;
        struct st_complex complex[2];
    }m_scan;

    struct st_textureGeo{
        int     lines;
        int     samples;
    }m_texture_geo = {
        256,
        512
    };

    QByteArray      mData;
};

RenderB::RenderB(QSize size) : program(0)
{
    mSize = size;
    mData.resize (size.width () * size.height ());

    memset (mData.data (), 255, size.width () * size.height ());
}


void RenderB::initialize()
{
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    initShaders();
    initData();

    createGeometry ();
}

void RenderB::createGeometry ()
{
    struct point
    {
        GLfloat x;
        GLfloat y;
    };

//    //make texture vertices and coord
//    GLint lines   = m_texture_geo.lines;
//    GLint samples = m_texture_geo.samples;
//    Q_UNUSED (samples);

//    GLfloat radius = m_probe.radius;
//    GLfloat depth  = m_scan.depth;

//    GLfloat theta  = m_probe.angle * 3.1415926 / 180/2;
//    GLfloat theta0 = m_probe.angle * 3.1415926 / 180 / (lines - 1);

//    GLfloat head = radius * cos(theta);
//    GLfloat scal = (radius + depth - head )/mSize.height ();

//    GLfloat ratex	= (mSize.width ()/2) * scal;
//    GLfloat ratey	= (depth + radius - radius * cos(theta))/2;

//    GLfloat delta = (depth + radius - radius*cos(theta)) / 2.0 + radius*cos(theta);


//    point pt[2];

//    GLfloat pt_x = 0.0;
//    GLfloat pt_y = 0.0;

//    for (int i = 0; i < m_scan.complex_len; i++) {
//        GLfloat angle;
//        angle = m_scan.complex[i].opengl_data_l.angle;
//        m_scan.complex[i].opengl_data_l.vertices.clear ();

//        for (int k = 0; k < lines; k++) {
//            //左下
//            pt[1].x = (GLfloat)-(radius + depth)*sin(theta-i*theta0) /ratex  ;
//            pt[1].y = (GLfloat)-((radius + depth)*cos(theta - i*theta0) - delta ) /ratey ;

//            //左上
//            pt[0].x = -radius*sin(theta - i*theta0) / ratex ;
//            pt[0].y =  (delta - radius*cos(theta - i*theta0)) /ratey;

//            pt_x = (pt[0].x - pt[1].x) * cos(-angle) - (pt[0].y - pt[1].y) * sin(-angle) + pt[1].x;
//            pt_y = (pt[0].y - pt[1].y) * cos(-angle) - (pt[0].x - pt[1].x) * sin(-angle) + pt[1].y;


//            //顶点
//            m_scan.complex[i].opengl_data_l.vertices << QVector2D(pt[0].x, pt[0].y)  \
//                        << QVector2D(pt[1].x, pt[1].y);
//            //纹理坐标
//            m_scan.complex[i].opengl_data_l.vercoords << QVector2D(0.0, 1.0 * i / (lines - 1))
//                        << QVector2D(1.0, 1.0 * i / (lines - 1));
//        }

//        angle = m_scan.complex[i].opengl_data_r.angle;
//        m_scan.complex[i].opengl_data_r.vertices.clear ();

//        for (int k = 0; k < lines; k++) {
//            //左下
//            pt[1].x = (GLfloat)-(radius + depth)*sin(theta-i*theta0) /ratex  ;
//            pt[1].y = (GLfloat)-((radius + depth)*cos(theta - i*theta0) - delta ) /ratey ;

//            //左上
//            pt[0].x = -radius*sin(theta - i*theta0) / ratex ;
//            pt[0].y =  (delta - radius*cos(theta - i*theta0)) /ratey;

//            pt_x = (pt[0].x - pt[1].x) * cos(-angle) - (pt[0].y - pt[1].y) * sin(-angle) + pt[1].x;
//            pt_y = (pt[0].y - pt[1].y) * cos(-angle) - (pt[0].x - pt[1].x) * sin(-angle) + pt[1].y;


//            //顶点
//            m_scan.complex[i].opengl_data_r.vertices << QVector2D(pt[0].x, pt[0].y)  \
//                        << QVector2D(pt_x, -pt_y);
//            //纹理坐标
//            m_scan.complex[i].opengl_data_r.vercoords << QVector2D(0.0, 1.0 * i / (lines - 1))
//                        << QVector2D(1.0, 1.0 * i / (lines - 1));
//        }

//        angle = m_scan.complex[i].opengl_data_m.angle;
//        m_scan.complex[i].opengl_data_r.vertices.clear ();

//        for (int k = 0; k < lines; k++) {
//            //左下
//            pt[1].x = (GLfloat)-(radius + depth)*sin(theta-i*theta0) /ratex  ;
//            pt[1].y = (GLfloat)-((radius + depth)*cos(theta - i*theta0) - delta ) /ratey ;

//            //左上
//            pt[0].x = -radius*sin(theta - i*theta0) / ratex ;
//            pt[0].y =  (delta - radius*cos(theta - i*theta0)) /ratey;

//            pt_x = (pt[0].x - pt[1].x) * cos(-angle) - (pt[0].y - pt[1].y) * sin(-angle) + pt[1].x;
//            pt_y = (pt[0].y - pt[1].y) * cos(-angle) - (pt[0].x - pt[1].x) * sin(-angle) + pt[1].y;


//            //顶点
//            m_scan.complex[i].opengl_data_r.vertices << QVector2D(pt[0].x, pt[0].y)  \
//                        << QVector2D(pt[1].x, pt[1].y);
//            //纹理坐标
//            m_scan.complex[i].opengl_data_r.vercoords << QVector2D(0.0, 1.0 * i / (lines - 1))
//                        << QVector2D(sin(angle), 1.0 * i / (lines - 1) + cos(angle));
//        }
//    }



    GLint lines   =  m_texture_geo.lines;
    GLint samples =  m_texture_geo.samples;
    int w = mSize.width ();
    int h = mSize.height ();
    float scanProbeWidth = m_probe.linewidth;
    float depth = m_scan.depth;

    int scanwidth = int ((double)h * scanProbeWidth/depth);
    int scandepth = int ((double)w * depth/scanProbeWidth);

    double deta0 = scanwidth / lines;

    double hscanwidth = scanwidth/2;
    int hw = w/2;

    point pt[2];

    Q_UNUSED (samples);
    Q_UNUSED (scandepth);

    for (int i = 0; i < m_scan.complex_len; i++) {
        GLfloat angle;
        angle = m_scan.complex[i].opengl_data_l.angle;
        m_scan.complex[i].opengl_data_l.vertices.clear ();
        m_scan.complex[i].opengl_data_l.vercoords.clear ();

        for (int i = 0; i < lines/2; i++) {
            pt[0].x = -((hscanwidth - i * deta0)/hw);
            pt[0].y = 1;

            pt[1].x = -((hscanwidth - i * deta0)/hw);
            pt[1].y = -1;
        }

        for (int i = lines/2; i < lines; i++) {
            pt[0].x = ((i * deta0 - hscanwidth)/hw);
            pt[0].y = 1;

            pt[1].x = ((i * deta0 - hscanwidth)/hw);
            pt[1].y = -1;

        }
    }
}

void RenderB::paint()
{
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable( GL_DEPTH_TEST );
    glEnable(GL_TEXTURE_2D);

    for (int i = 0; i < m_scan.complex_len; i++) {
        glActiveTexture (GL_TEXTURE0 +  3*i);
        glBindTexture (GL_TEXTURE_2D, m_scan.complex[i].opengl_data_l.texture->textureId ());

        glActiveTexture (GL_TEXTURE0 + 1 + 3*i);
        glBindTexture (GL_TEXTURE_2D, m_scan.complex[i].opengl_data_m.texture->textureId ());

        glActiveTexture (GL_TEXTURE0 + 2 + 3*i);
        glBindTexture (GL_TEXTURE_2D, m_scan.complex[i].opengl_data_r.texture->textureId ());
    }

    glActiveTexture (GL_TEXTURE0);

    program.bind();
    drawDSC();
    program.release();

    for (int i = 0; i < m_scan.complex_len; i++) {
        m_scan.complex[i].opengl_data_l.texture->release ();
        m_scan.complex[i].opengl_data_m.texture->release ();
        m_scan.complex[i].opengl_data_r.texture->release ();
    }


}

void RenderB::setMap (int id, unsigned char *map, int size)
{
    Q_UNUSED (id);

    colorsMapB.clear ();
    for (int i = 0; i < size; i += 3) {
        colorsMapB << QVector3D(map[i], map[i+1], map[i+2]);
    }
}

void RenderB::initShaders()
{
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    QOpenGLShader *vShader = new QOpenGLShader(QOpenGLShader::Vertex,  &program);
    const char *vsrc = \
        "#ifdef GL_ES\n"
        "precision highp int;\n"
        "precision highp float;\n"
        "#endif\n"
        "uniform mat4 u_model;\n"
        "uniform mat4 u_view;\n"
        "uniform mat4 u_projection;\n"
        "attribute  vec4 a_Position[5];\n"
        "attribute  vec2 a_texCoord[5];\n"
        "varying    vec2 v_texCoord[5];\n"
        "varying    vec3 v_fragPos [5];\n"
        "void main()\n"
        "{\n"
            "int num;\n"
            "for (num = 0; num < 5; num += 1) {\n"
                v_texCoord =
    }
            "v_fragPos   = vec3(u_model * a_Position);"
            "v_texCoord  = a_texCoord;\n"
            "gl_Position = u_projection * u_view * u_model * a_Position[1];\n"
        "}\n";
    vShader->compileSourceCode(vsrc);

    QOpenGLShader *fShader = new QOpenGLShader(QOpenGLShader::Fragment, &program);
    const char *fsrc =
        "#ifdef GL_ES\n"
        "precision mediump int;\n"
        "precision mediump float;\n"
        "#endif\n"
        "uniform  highp vec3 color_value[256];\n"
        "varying  highp vec2      v_texCoord[5];\n"
        "uniform  highp sampler2D s_texture[5];\n"
        "uniform  float cut;"
        "varying  vec3  v_fragPos[5];\n"
        "void main(void)\n"
        "{\n"
            "float index;\n"
            "vec3 color;\n"
            "vec4 texture[5] =  texture2D(s_texture, v_texCoord) ;\n"
//            "index = texture.x * 255.0;\n"
//            "int t = int(index);\n"
//            "if (-cut < v_fragPos.x && v_fragPos.x < cut) {\n"
//                "color = color_value[t] / 255.0;\n"
//            "}else{\n"
//                "color = vec3(0, 0, 0);\n"
//            "}\n"
//            "gl_FragColor = vec4(color, 1.0);\n"
            "gl_FragColor = texture;\n"
        "}\n";

    fShader->compileSourceCode(fsrc);

    program.addShader(vShader);
    program.addShader(fShader);
    program.link();

    program.bindAttributeLocation("a_Position", 0);
    program.bindAttributeLocation("a_texCoord", 1);
    program.bindAttributeLocation("a_Color",    2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

}

QOpenGLTexture *RenderB::getTextures ()
{
    QOpenGLTexture *texture = new QOpenGLTexture(QOpenGLTexture::Target2D);

    texture->setSize(mSize.width (), mSize.height ());
    texture->setFormat(QOpenGLTexture::LuminanceFormat);
    texture->allocateStorage(QOpenGLTexture::Luminance, QOpenGLTexture::UInt8);

    texture->setMinificationFilter(QOpenGLTexture::Linear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

    texture->setWrapMode(QOpenGLTexture::Repeat);

    return texture;
}

void RenderB::initData()
{
    //set model matrix
    mModelMatrix.setToIdentity();
    //set view matrix
    mViewMatrix.setToIdentity();
    mViewMatrix.lookAt(QVector3D(0.0f, 0.0f, 1.00f), QVector3D(0.0f, 0.0f, -5.0f), QVector3D(0.0f, 1.0f, 0.0f));

    //set projection matrix
    float bottom=  -1.0f;
    float top	=	1.0f;
    float n		=	1.0f;
    float f		=	100.0f;
    //FIXME 1 投影矩阵 左右值取-1, 1，不要让图像被缩放
    mProjectionMatrix.setToIdentity();
    mProjectionMatrix.frustum(-1.0, 1.0, bottom, top, n, f);
}

void RenderB::drawDSC()
{
    mModelMatrixHandle	= program.uniformLocation("u_model");
    mViewMatrixHandle	= program.uniformLocation("u_view");
    mProjectionMatrixHandle = program.uniformLocation("u_projection");

    mPositionHandle  = program.attributeLocation("a_Position");
    mTexCoordHandle	 = program.attributeLocation("a_texCoord");
    mColorArrayhandle = program.uniformLocation("color_value");


    program.setUniformValue("s_texture", 0);
//    program.enableAttributeArray(mPositionHandle);
//    program.setAttributeArray(mPositionHandle, vertices.constData());
//    program.enableAttributeArray(mTexCoordHandle);
//    program.setAttributeArray(mTexCoordHandle, vercoords.constData());

//    program.setUniformValue(mModelMatrixHandle, mModelMatrix);
//    program.setUniformValue(mViewMatrixHandle, mViewMatrix);
//    program.setUniformValue(mProjectionMatrixHandle, mProjectionMatrix);

//    program.setUniformValue("s_texture", 0);

//    program.setUniformValueArray(mColorArrayhandle,  colorsMapB.constData(), 256);

//    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size());

    program.disableAttributeArray(mPositionHandle);
    program.disableAttributeArray(mTexCoordHandle);
}

void RenderB::setScan (QDomNode root)
{
    if (root.isNull ()) return;

    QDomNode node;

    node = root.firstChildElement ("depth");
    if (!node.isNull ()) {
        float value = node.toElement ().attribute("value", "1.0").toFloat ();
        m_scan.depth = value;
        createGeometry ();
    }

    node = root.firstChildElement ("complex");
    if (!node.isNull ()) {
        int len = node.toElement ().attribute ("len", "0").toInt ();

        for (int i = 0; i < len; i++){
            QDomNode L = node.toElement ().firstChildElement (QString("L%1").arg (i + 1));
            QDomNode R = node.toElement ().firstChildElement (QString("R%1").arg (i + 1));
            QDomNode M = node.toElement ().firstChildElement (QString("M%1").arg (i + 1));

            if (L.isNull () || R.isNull ()) continue;

            m_scan.complex[i].opengl_data_l.angle = L.toElement ().attribute ("angle").toFloat ();
            m_scan.complex[i].opengl_data_r.angle = R.toElement ().attribute ("angle").toFloat ();
            m_scan.complex[i].opengl_data_m.angle = M.toElement ().attribute ("angle").toFloat ();

            m_scan.complex[i].opengl_data_l.flag = L.toElement ().attribute ("flag").toInt ();
            m_scan.complex[i].opengl_data_r.flag = R.toElement ().attribute ("flag").toInt ();
            m_scan.complex[i].opengl_data_m.flag = M.toElement ().attribute ("flag").toInt ();

            m_scan.complex[i].opengl_data_l.texture = getTextures ();
            m_scan.complex[i].opengl_data_r.texture = getTextures ();
            m_scan.complex[i].opengl_data_m.texture = getTextures ();


            m_scan.complex[i].opengl_data_r.vertices.clear ();
            m_scan.complex[i].opengl_data_m.vertices.clear ();
            m_scan.complex[i].opengl_data_l.vertices.clear ();

            m_scan.complex[i].opengl_data_r.vercoords.clear ();
            m_scan.complex[i].opengl_data_l.vercoords.clear ();
            m_scan.complex[i].opengl_data_m.vercoords.clear ();
        }
    }
}

void RenderB::addData (int, unsigned char *data, int size)
{
    mData.resize (size);
    memcpy( mData.data (), data, sizeof(unsigned char) * (size_t)size);
}

class RenderThread;
class RenderItem : public QQuickItem
{
    Q_OBJECT

public:
    RenderItem(QDomNode, QQuickItem *parent = 0);

    static QList<QThread *> threads;

public Q_SLOTS:
    void ready();

protected:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);

private:
    RenderThread *m_renderThread;

Q_SIGNALS:
    void setMap   ( int, unsigned char *, int );
    void addData  ( int, unsigned char *, int );
    void setScan  (QDomNode);
};

QList<QThread *> RenderItem::threads;

class RenderThread : public QThread
{
    Q_OBJECT
public:
    /*
        <render>
            <mode  value="B">
                <B x= "-1" y = "-1" w = "2" h = "2"/>
            </mode>
            <probe name="R10" icon="res/R10.png" hard="16" soft="3" type="Convex" element="128" interval="1.175"
radius="1.0" Angle="150.4"/>
            <size width=768 height=512/>
        </render>

    */
    RenderThread(QDomNode root)
        : surface(0)
        , context(0)
        , m_renderFbo(0)
        , m_displayFbo(0)
        , m_size(QSize(0,0))
    {

        QDomNode node;

        node = root.firstChildElement (QString("mode"));
        if (node.isNull ()) {
            qWarning() << __FILE__ << __FUNCTION__ << "mode not found";
            return;
        }

        m_mode_string = node.toElement().attribute("value");

        node = root.firstChildElement (QString("size"));
        if (node.isNull ()) {
            qWarning() << __FILE__ << __FUNCTION__ << "size not found";
            return;
        }
        m_size.setWidth  (node.toElement ().attribute ("width", "768").toInt ());
        m_size.setHeight (node.toElement ().attribute ("height", "512").toInt ());

        m_root_config = root.cloneNode (true);

        RenderItem::threads << this;
    }

    QOffscreenSurface *surface;
    QOpenGLContext    *context;

public slots:
    void renderNext()
    {
        context->makeCurrent(surface);

        if (!m_renderFbo) {
            // Initialize the buffers and renderer
            QOpenGLFramebufferObjectFormat format;
            format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
            m_renderFbo  = new QOpenGLFramebufferObject (m_size, format);
            m_displayFbo = new QOpenGLFramebufferObject (m_size, format);

            if (m_mode_string == "B") {
                m_render = new RenderB(m_size);

                QDomNode node;

                node = m_root_config.firstChildElement ("probe");
                m_render->setGeoProbe (node);

                node = m_root_config.firstChildElement ("scan");
                m_render->setScan (node);


            }

            m_render->initialize();
        }

        m_renderFbo->bind();
        context->functions ()->glViewport (0, 0, m_size.width(), m_size.height());

        m_render->paint();

        context->functions()->glFlush();

        m_renderFbo->bindDefault();
        qSwap(m_renderFbo, m_displayFbo);

        emit textureReady((int)m_displayFbo->texture(), m_size);
    }

    void shutDown()
    {
        context->makeCurrent(surface);
        delete m_renderFbo;
        delete m_displayFbo;
        delete m_render;
        context->doneCurrent();
        delete context;

        // schedule this to be deleted only after we're done cleaning up
        surface->deleteLater();

        // Stop event processing, move the thread to GUI and make sure it is deleted.
        exit();
        moveToThread(QGuiApplication::instance()->thread());
    }

    void setMap   ( int id, unsigned char *map, int size ){
        m_render->setMap (id, map, size);

    }

    void addData  ( int id, unsigned char *buf, int size){
        m_render->addData (id, buf, size);
    }

    void setScan  ( QDomNode node){
        m_render->setScan (node);
    }

signals:
    void textureReady(int id, const QSize &size);

private:
    QOpenGLFramebufferObject *m_renderFbo;
    QOpenGLFramebufferObject *m_displayFbo;

    RenderBase                *m_render;

    QDomNode                   m_root_config;
    QString                    m_mode_string;
    QSize m_size;
};

class TextureNode : public QObject, public QSGSimpleTextureNode
{
    Q_OBJECT

public:
    TextureNode(QQuickWindow *window)
        : m_id(0)
        , m_size(0, 0)
        , m_texture(0)
        , m_window(window)
    {
        m_texture = m_window->createTextureFromId(0, QSize(1, 1));
        setTexture(m_texture);
        setFiltering(QSGTexture::Linear);
    }

    ~TextureNode()
    {
        delete m_texture;
    }

signals:
    void textureInUse();
    void pendingNewTexture();

public slots:

    void newTexture(int id, const QSize &size) {
        m_mutex.lock();
        m_id = id;
        m_size = size;
        m_mutex.unlock();

        emit pendingNewTexture();
    }


    // Before the scene graph starts to render, we update to the pending texture
    void prepareNode() {
        m_mutex.lock();
        int newId = m_id;
        QSize size = m_size;
        m_id = 0;
        m_mutex.unlock();
        if (newId) {
            delete m_texture;

            m_texture = m_window->createTextureFromId((uint)newId, size);
            setTexture(m_texture);

            markDirty(DirtyMaterial);

            // This will notify the rendering thread that the texture is now being rendered
            // and it can start rendering to the other one.
            emit textureInUse();
        }
    }

private:

    QMutex m_mutex;
    int m_id;

    QSize m_size;

    QSGTexture   *m_texture;
    QQuickWindow *m_window;

};

/*
    <RenderItem>
        <render>
            ...
        <render>
    </RenderItem>
*/
RenderItem::RenderItem(QDomNode root, QQuickItem *parent)
    : QQuickItem(parent)
    , m_renderThread(0)
{
    setSize (parent->boundingRect ().toRect ().size ());
    setFlag(ItemHasContents, true);

    if (root.toElement ().tagName () != QString("RenderItem")) {
        qWarning() << __FUNCTION__ << "DomNode format's err";
        return;
    }

    QDomNode node = root.toElement ().firstChildElement ("render");

    m_renderThread = new RenderThread(node);

}

void RenderItem::ready()
{
    m_renderThread->surface = new QOffscreenSurface();
    m_renderThread->surface->setFormat(m_renderThread->context->format());
    m_renderThread->surface->create();

    m_renderThread->moveToThread(m_renderThread);

    connect(window(), &QQuickWindow::sceneGraphInvalidated, m_renderThread, &RenderThread::shutDown, Qt::QueuedConnection);

    connect(this, SIGNAL(setMap(int, unsigned char *, int)), m_renderThread, SLOT(setMap (int, unsigned char *, int)));
    connect(this, SIGNAL(addData(int, unsigned char *, int)),m_renderThread, SLOT(addData(int, unsigned char *, int)));
    connect(this, SIGNAL(setScan(QDomNode)),                 m_renderThread, SLOT(setScan(QDomNode)));

    m_renderThread->start();
    update();
}

QSGNode *RenderItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    TextureNode *node = static_cast<TextureNode *>(oldNode);

    if (!m_renderThread->context) {
        QOpenGLContext *current = window()->openglContext();

        current->doneCurrent();

        m_renderThread->context = new QOpenGLContext();
        m_renderThread->context->setFormat(current->format());
        m_renderThread->context->setShareContext(current);
        m_renderThread->context->create();
        m_renderThread->context->moveToThread(m_renderThread);

        current->makeCurrent(window());
        QMetaObject::invokeMethod(this, "ready");
        return 0;
    }

    if (!node) {
        node = new TextureNode(window());

        connect(m_renderThread, &RenderThread::textureReady, node, &TextureNode::newTexture, Qt::DirectConnection);
        connect(node, &TextureNode::pendingNewTexture, window(), &QQuickWindow::update, Qt::QueuedConnection);
        connect(window(), &QQuickWindow::beforeRendering, node, &TextureNode::prepareNode, Qt::DirectConnection);
        connect(node, &TextureNode::textureInUse, m_renderThread, &RenderThread::renderNext, Qt::QueuedConnection);

        // Get the production of FBO textures started..
        QMetaObject::invokeMethod(m_renderThread, "renderNext", Qt::QueuedConnection);
    }

    node->setRect(boundingRect());

    return node;
}



#define CHECK_DIVISOR(var) do { \
	if ((var) == 0) { \
	qDebug() << __FILE__<<__LINE__<< "if you want to div 0, it's a thrilling thing."; \
	} \
	}while(0);

VirtualBox::VirtualBox(int id \
                     ,  QDomDocument     &machine   \
                     ,  QDomDocument     &deviceXml \
                     ,  QDomDocument     &probeXml  \
                     ,  QDomDocument     &softXml   \
                     ,  QDomDocument     &configXml \
                     , QQuickItem *gui)
{

    m_id = id;

	/*member variables assignment*/
    m_machine_xml= machine.cloneNode ().toDocument ();
    m_probe_xml  = probeXml.cloneNode ().toDocument ();
	m_config_xml = configXml.cloneNode().toDocument();
	m_gui_parent = gui;

	/*engine init*/
	m_engine = new QScriptEngine;

    m_ultrasound  =  Ultrasound::newDevice (m_machine_xml, \
                                            deviceXml,  \
                                            softXml,    \
                                            m_engine);

    jsLoader ();
    functionLoader ();

    m_render_ui = new RenderItem(m_config_xml.documentElement (), m_gui_parent);
    EpluginsManager::instance ()->newItems (m_gui_parent, m_engine, id);
}


VirtualBox::~VirtualBox (){

}

QScriptValue VirtualBox::callerFunction(QString function)
{
	QScriptValue ret;
	QScriptValue caller = m_engine->globalObject().property(function);

	if (!caller.isValid()) {
		return caller;
	}else {
		ret = caller.call(QScriptValue());
	}
	return ret;
}

QScriptValue VirtualBox::callerFpgaFunction(QString var, QString value)
{
	m_fpga_control->setObjectValueFName(var, value);
	m_ultrasound->updateCtrlTable(true);

	return QScriptValue();
}

QScriptValue VirtualBox::callerSoftFunction(QString var, QString value)
{
	m_soft_control->setObjectValueFName(var, value);
	return QScriptValue();
}

void VirtualBox::functionLoader()
{
    /*add instance function */
    if (m_engine->hasUncaughtException ()){
        qWarning() << "Error:" << __FILE__ << __LINE__ << "StatusBar " << m_engine->uncaughtExceptionLineNumber () << " " << m_engine->uncaughtException ().toString ();
    }

}

void VirtualBox::jsLoader()
{

    /* add third js folder*/
    QDir pluginsDir(qApp->applicationDirPath() + "/third/js");

    foreach(QString fileName, pluginsDir.entryList(QDir::Files)) {
        QFile file(QDir::currentPath() + "/third/js" + fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        QByteArray content =  file.readAll();
        file.close ();

        m_engine->evaluate (content);

        if(m_engine->hasUncaughtException()) {
            qDebug() << "Error:" << "third/js/" + fileName << "jsLoader: evaluate() "<< m_engine->uncaughtExceptionLineNumber() \
                     << " "<< m_engine->uncaughtException ().toString ();
            m_engine->clearExceptions();
            exit(0);
        }
    }
}

QScriptEngine *VirtualBox::engine()
{
	return m_engine;
}

int   VirtualBox::dscrecth () const
{
    return m_dsc_image_h;
}

int VirtualBox::dscrectw () const
{
    return m_dsc_image_w;
}

int VirtualBox::dscrectx () const
{
    return m_dsc_image_x;
}

int VirtualBox::dscrecty () const
{
    return m_dsc_image_y;
}

QRect VirtualBox::dscRect() const
{
    return QRect(m_dsc_image_x, m_dsc_image_y, \
                 m_dsc_image_w, m_dsc_image_h);
}

void VirtualBox::setdscRect (QRect rect)
{
    if (rect == dscRect()) {
        return;
    }

    m_dsc_image_x = rect.x ();
    m_dsc_image_y = rect.y ();
    m_dsc_image_w = rect.width ();
    m_dsc_image_h = rect.height ();

    emit dscRectChanged ();
}

int VirtualBox::expectW() const
{
	return m_expect_dsc_w;
}
int VirtualBox::expectH() const
{
	return m_expect_dsc_h;
}
int VirtualBox::expectX() const
{
    return m_expect_dsc_x;
}
int VirtualBox::expectY() const
{
    return m_expect_dsc_y;
}

void VirtualBox::setExpectSize(int w, int h)
{
	m_expect_dsc_w = w;
	m_expect_dsc_h = h;
}

void VirtualBox::setExpectPos(int x, int y)
{
    m_expect_dsc_x = x;
    m_expect_dsc_y = y;
}


int VirtualBox::id ()  const
{
    return m_id;
}

#include "virtual_box.moc"
