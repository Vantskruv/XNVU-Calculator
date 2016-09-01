#ifndef QFLIGHTPLANTABLE_H
#define QFLIGHTPLANTABLE_H

#include <QTableWidget>

class QFlightplanTable : public QTableWidget
{
    public:
        void mousePressEvent(QMouseEvent* event);
        /*
        void setRow(int row, const QList<QTableWidgetItem*>& rowItems);
        QList<QTableWidgetItem*> takeRow(int);
        void move(bool);
        */
        QFlightplanTable(QWidget*&w);
};

#endif // QFLIGHTPLANTABLE_H
