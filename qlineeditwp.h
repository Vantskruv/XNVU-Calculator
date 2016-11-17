#ifndef QLINEEDITWP_H
#define QLINEEDITWP_H

#include <QLineEdit>
#include "NVU.h"

class QLineEditWP : public QLineEdit
{
    Q_OBJECT
public:
    explicit QLineEditWP(QWidget*&w);

signals:
    void clicked(QLineEditWP*);

public slots:

protected:
    //void focusInEvent(QFocusEvent* ev);
    void mousePressEvent(QMouseEvent* ev);
private:
    NVUPOINT* wp;
public:
    void setWaypoint(NVUPOINT* _wp);
    NVUPOINT* getWaypoint();
    virtual ~QLineEditWP();

};

#endif // QLINEEDITWP_H
