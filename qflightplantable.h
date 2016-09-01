#ifndef QFLIGHTPLANTABLE_H
#define QFLIGHTPLANTABLE_H

#include <QTableWidget>
#include <list>
#include <NVU.h>

class QFlightplanTable : public QTableWidget
{
    public:
        //std::list<NVUPOINT*> lNVU;
        //double fork;
        //double totalDistance;

        /*
        void insertWaypoint(NVUPOINT* wp, int row);
        void replaceWaypoint(NVUPOINT* wp, int row);
        void moveWaypoint(int row, bool down);
        void deleteWaypoint(int row);
        void clearFlightplan();
        void initializeFlightplan();
        */

        void mousePressEvent(QMouseEvent* event);
        /*
        void setRow(int row, const QList<QTableWidgetItem*>& rowItems);
        QList<QTableWidgetItem*> takeRow(int);
        void move(bool);
        */
        QFlightplanTable(QWidget*&w);
};

#endif // QFLIGHTPLANTABLE_H
