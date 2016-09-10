#ifndef QFLIGHTPLANTABLE_H
#define QFLIGHTPLANTABLE_H

#include <QTableWidget>
#include <QLabel>
#include <list>
#include <NVU.h>

class QFlightplanTable : public QTableWidget
{
    public:
        int dat;
        QLabel* qFork = NULL;
        bool showMeters = true;

        void deleteWaypoint(int row);                                   //Delete waypoint at row
        void insertWaypoint(NVUPOINT* wp, int row, int offset);         //Insert a waypoint with an offset value set. Before row (-1), after row (1), or replace the row (0).
        void insertRoute(std::vector<NVUPOINT*>, int row, int offset);  //Insert a list of waypoints, before, after or replace and then insert after a row.
        void insertWaypoint(NVUPOINT* wp, int row);                     //Insert a waypoint before the row.
        void moveWaypoint(int row, bool up);                            //Move waypoint at row up or down
        void refreshRow(int row, NVUPOINT* waypoint = NULL);                  //Refresh contents of a row with data from a waypoint, if waypoint is NULL, refresh current data.
        void refreshFlightplan();                                       //Refresh all waypoints
        void updateDistanceAndN();                                      //Updates distances, waypoint numbers and fork.
        void clearFlightplan();                                         //Delete all waypoints and clear the flightplan

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
