#include "qframeclickable.h"
#include <QDebug>

QFrameClickable::QFrameClickable(QWidget *parent) : QFrame(parent)
{

}

QFrameClickable::~QFrameClickable(){};

void QFrameClickable::mousePressEvent(QMouseEvent* ev)
{
    emit clicked(ev->pos());
}


