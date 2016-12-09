#include "mouse.h"

#include <QEvent>
#include <QDebug>
#include <QCursor>
#include <QGuiApplication>
#include <QScreen>
#include <QMouseEvent>


Mouse::Mouse(QObject *host): m_state(false) {
    Q_UNUSED(host);
}

Mouse::~Mouse() {
    QGuiApplication::instance()->removeEventFilter(this);
}

bool Mouse::lock() {
    if (m_state == false){

        QRect  rect = QGuiApplication::primaryScreen()->geometry();
        QCursor::setPos(rect.width()/2, rect.height()/2);


        QGuiApplication::setOverrideCursor(Qt::BlankCursor);
        qApp->installEventFilter(this);
        m_state = true;
        return m_state;
    }else{
        return false;
    }

}

void Mouse::unlock() {
    if (m_state == true){
        m_state = false;
        QGuiApplication::instance()->removeEventFilter(this);
        QGuiApplication::restoreOverrideCursor();
    }
}

bool  Mouse::state() const{
    return m_state;
}


bool Mouse::eventFilter(QObject *obj, QEvent *event){
    Q_UNUSED(obj);
    static QRect  rect = QGuiApplication::primaryScreen()->geometry();
    static QPoint oldPoint( rect.width()/2, rect.height()/2);

    if (event->type() == QEvent::MouseMove){
        QPoint deff =  QCursor::pos() - oldPoint;
        QCursor::setPos(rect.width()/2, rect.height()/2);
        if (deff.x() == 0 && deff.y() == 0) return true;
        emit mouseMove(deff);
        emit mousePoint(deff.x(), deff.y());
        return true;
    } else if (event->type() == QEvent::MouseButtonRelease){
        static bool press = false;
        QMouseEvent *mouse = (QMouseEvent *)(event);
        press = press ? false:true;
        if (mouse->button() == Qt::LeftButton &&  press == true){
            emit mouseLeftClick();
        }else if (mouse->button() == Qt::RightButton && press == true){
            emit mouseRightClick();
        }
    }

    return false;
}
