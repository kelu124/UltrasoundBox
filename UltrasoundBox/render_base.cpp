#include "render_base.h"

#include <QFile>

QByteArray RenderBase::readFile (QString &n)
{
    QFile file(n);
    if (!file.open (QFile::ReadOnly)) {
        return QByteArray();
    }

    QByteArray data = file.readAll ();

    file.close ();
    return data;
}
void RenderBase::loadResure (QDomNode root)
{
    if (root.isNull ()) return;

    qDebug() << root.toElement ().tagName ();

    QDomNode node = root.firstChildElement ();

    while(!node.isNull ()) {
        if (node.isElement ()) {
            if (node.toElement ().tagName () == QString("b")) {
                QString name = node.toElement ().attribute ("value", QString("0"));
                QByteArray data = readFile (name);

                if (!data.isEmpty ()) {
                    for (int i = 0; i < 8; i++) {
                        for (int j = 0; j < 256; j++) {
                            char ch = data.at (i * 256 + j);
                            m_img_trans.colormap.b_colormaplist[i][j][0] = static_cast<unsigned char>(ch);
                            m_img_trans.colormap.b_colormaplist[i][j][1] = static_cast<unsigned char>(ch);
                            m_img_trans.colormap.b_colormaplist[i][j][2] = static_cast<unsigned char>(ch);
                        }
                    }
                }
            }else if (node.toElement ().tagName () == QString("c")){
                /*add you  code*/
            }else if (node.toElement ().tagName () == QString("d")) {
                /*add your code*/
            }else if (node.toElement ().tagName () == QString("m")) {
                /*add your code*/
            }
        }

        node = node.nextSiblingElement ();
    }

}

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

