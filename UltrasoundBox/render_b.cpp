#include "render_b.h"

RenderB::RenderB(QSize size) : program(0)
{
    mSize = size;
    mByteArrayData.resize (size.width () * size.height ());

    memset (mByteArrayData.data (),  0, \
            static_cast<size_t>(size.width () * size.height ()));

    colorsMapB.clear ();
    for (int i = 0; i < 256; i++) {
        colorsMapB << QVector3D(i, i, i);
    }

    m_texture_geo.lines   = 256;
    m_texture_geo.samples = 512;

    m_img_trans.grayReverse = 0;

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
    mVerticesB  << QVector3D(-1, -1, 0.0f) \
                << QVector3D( 1, -1, 0.0f) \
                << QVector3D( 1,  1, 0.0f) \
                << QVector3D(-1,  1, 0.0f);

    mTexcoordsB << QVector2D (0 , 0) \
                 << QVector2D (1 , 0) \
                 << QVector2D (1,  0) \
                 << QVector2D (0,  1);
    mViewMatrix.setToIdentity ();
    mViewMatrix.lookAt (QVector3D (0.0f, 0.0f, 1.001f), QVector3D(0.0f, 0.0f, -5.0f), QVector3D(0.0f, 1.0f, 0.0f));

}

void RenderB::paint()
{

    mMVPMatrixHandle = program.uniformLocation("u_MVPMatrix");
    mPositionHandle  = program.attributeLocation("a_Position");
    mTexCoordHandle	 = program.attributeLocation("a_texCoord");
    mColorArrayhandle = program.uniformLocation("color_value");

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable( GL_DEPTH_TEST );
    glEnable(GL_TEXTURE_2D);

    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, m_scan.mData.texture->textureId ());

    m_scan.mData.texture->bind();
    program.bind();
    drawDSC();
    program.release();
    m_scan.mData.texture->release ();
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
//    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    program.addShaderFromSourceFile (QOpenGLShader::Vertex,   ":/glsl/shaderv.glsl");
    program.addShaderFromSourceFile (QOpenGLShader::Fragment, ":/glsl/shaderf.glsl");
    program.link();

    program.bindAttributeLocation("a_Position", 0);
    program.bindAttributeLocation("a_texCoord", 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    program.bind ();

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

    m_scan.mData.texture = getTextures ();
}

void RenderB::drawDSC()
{
    program.enableAttributeArray(mTexCoordHandle);
    program.setAttributeArray(mTexCoordHandle,  mTexcoordsB.constData());

    mMVPMatrix = mProjectionMatrix * mViewMatrix * mModelMatrix;

    program.setUniformValue (mMVPMatrixHandle, mMVPMatrix);
    program.setUniformValueArray (mColorArrayhandle, colorsMapB.constData (), colorsMapB.size ());

    program.setUniformValue ("grayReverse", static_cast<GLint>(m_img_trans.grayReverse));

    m_mutex.lock ();
    m_scan.mData.texture->setData (QOpenGLTexture::Luminance, QOpenGLTexture::UInt8, mByteArrayData.data ());
    m_mutex.unlock ();

    int texHandle = program.uniformLocation ("s_texture");
    program.setUniformValue (texHandle, 0);

    program.enableAttributeArray(mPositionHandle);
    program.setAttributeArray (mPositionHandle, mVerticesB.constData ());

    glDrawArrays (GL_TRIANGLE_FAN, 0, mVerticesB.size ());

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
}

void RenderB::addData (int,  char *data, int size)
{

//    m_mutex.lock ();
    mByteArrayData.resize (size);
    memcpy( mByteArrayData.data (), data, \
            static_cast<size_t>(sizeof(unsigned char) * static_cast<unsigned long>(size)));
//    m_mutex.unlock ();
}

