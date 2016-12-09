#include "io_controls.h"
#include "IO/mouse/mouse.h"

class IOControlsPrivate
{
public:
    IOControlsPrivate(IOControls *qptr): \
        q_ptr(qptr),
        mouse(new Mouse(0)){
    }

    ~IOControlsPrivate() {}

    IOControls   *q_ptr;
    Mouse        *mouse;
};

IOControls::IOControls(): m_dptr(new IOControlsPrivate(this))
{
    connect (m_dptr->mouse, SIGNAL(mousePoint(int,int)), this, SIGNAL(moveXY(int,int)));
    connect (m_dptr->mouse, SIGNAL(mouseMove(QPoint)), this, SIGNAL(move(QPoint)));
    connect (m_dptr->mouse, SIGNAL(mouseLeftClick()),  this, SIGNAL(leftClicked()));
    connect (m_dptr->mouse, SIGNAL(mouseRightClick()), this, SIGNAL(rightClicked()));
}

IOControls::~IOControls ()
{

}

bool IOControls::state() const
{
    return m_dptr->mouse->state ();
}


void IOControls::setState(bool flag)
{
    if (flag == m_dptr->mouse->state ()){
        return;
    }

    if (!flag == false)
        m_dptr->mouse->lock ();
    else
        m_dptr->mouse->unlock ();

    emit stateChanged();
}
