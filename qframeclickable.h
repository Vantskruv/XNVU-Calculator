#ifndef QFRAMECLICKABLE_H
#define QFRAMECLICKABLE_H

#include <QFrame>
#include <QMouseEvent>

class QFrameClickable : public QFrame
{
    Q_OBJECT
    public:
    QFrameClickable(QWidget*&parent);
    ~QFrameClickable();

    protected:
        void mousePressEvent(QMouseEvent *ev);

    signals:
        void clicked(QPoint pos);

    public:


};

#endif // QFRAMECLICKABLE_H
