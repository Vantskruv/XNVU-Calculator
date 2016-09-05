#include "qframeclickable.h"

QFrameClickable::QFrameClickable(QWidget *parent) : QFrame(parent)
{

}

QFrameClickable::~QFrameClickable(){};

void QFrameClickable::mousePressEvent(QMouseEvent* ev)
{
    emit clicked(ev->pos());
}


