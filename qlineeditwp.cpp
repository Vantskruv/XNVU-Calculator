#include "qlineeditwp.h"
#include <QFocusEvent>

QLineEditWP::QLineEditWP(QWidget *&w) : QLineEdit(w)
{
    wp = NULL;
}

QLineEditWP::~QLineEditWP()
{
    if(wp!=NULL) delete wp;
    wp = NULL;
}

void QLineEditWP::setWaypoint(NVUPOINT* _wp)
{
    if(wp!=NULL) delete wp;
    if(_wp)
    {
        wp = new NVUPOINT(*_wp);
        setText(wp->name);
    }
    else
    {
        wp = NULL;
        setText("");
    }
}

NVUPOINT* QLineEditWP::getWaypoint()
{
    return wp;
}

void QLineEditWP::mousePressEvent(QMouseEvent *ev)
{
    emit clicked(this);
}

/*
void QLineEditWP::focusInEvent(QFocusEvent *ev)
{
    if(ev->reason() == Qt::MouseFocusReason)
    {

    }

    QLineEdit::focusInEvent(ev);
}
*/
