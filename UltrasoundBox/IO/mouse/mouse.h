#ifndef MOUSE_H
#define MOUSE_H
#include <QObject>
#include <QPoint>

class Mouse : public QObject
{
    Q_OBJECT
public:
    Mouse(QObject *host);
    ~Mouse();

   bool lock();
   void unlock();

   bool  state() const;
protected:
    bool eventFilter(QObject *obj, QEvent *event);

Q_SIGNALS:
    void mousePoint(int, int);
    void mouseMove(QPoint);
    void mouseLeftClick();
    void mouseRightClick();

private:
    bool m_state;
};

#endif // Mouse_H
