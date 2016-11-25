#ifndef RENDERITEM_H
#define RENDERITEM_H

#include <QDomDocument>
#include <QQuickFramebufferObject>
#include "render_b.h"

class RenderItem : public QQuickFramebufferObject
{
    Q_OBJECT
public:
    RenderItem(QDomNode, QQuickItem *);

    Renderer *createRenderer() const;
    QDomNode    paramsNode() const;
    QByteArray  dataNode() const;


    void  addData(QByteArray);
    void  setParams( QDomNode);
private:
    QDomDocument        m_config_doc;

    QDomNode            m_render_params;
    QByteArray          m_render_data;

Q_SIGNALS:
    void add (int, char *, int);

public Q_SLOTS:
    void addD();
};


#endif // RENDERITEM_H
