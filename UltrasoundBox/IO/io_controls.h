#ifndef IO_CONTROLS_H
#define IO_CONTROLS_H

#include <QObject>
#include <QPoint>
#include <QDebug>
class IOControlsPrivate;
class IOControls:public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool state READ state WRITE setState NOTIFY stateChanged)
public:
	static IOControls *instance(){
        static IOControls *_this = NULL;
		if (!_this) {
			_this = new IOControls;
		}

		return _this;
	}

    ~IOControls();

    bool  state() const;
    void  setState(bool);

Q_SIGNALS:
    void move(QPoint);
    void leftClicked();
    void rightClicked();
    void moveXY(int, int);

    void stateChanged();


private:
        IOControls();
        IOControlsPrivate *m_dptr;

};

#endif // IO_CONTROLS_H
