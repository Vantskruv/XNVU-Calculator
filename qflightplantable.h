#ifndef QFLIGHTPLANTABLE_H
#define QFLIGHTPLANTABLE_H

#include <QTableWidget>
#include <QLabel>
#include <list>
#include <NVU.h>

#define ENUM(name, ...)()

class QFlightplanTable : public QTableWidget
{
    private:
        void refreshRow(int row);            //Refresh contents of a row with data from a waypoint, if waypoint is NULL, refresh current data.
        std::vector<NVUPOINT*> lNVUPoints;

    public:
        enum COL{N, ID, TYPE, ALT, LAT, LON, MD, OZMPUV, OZMPUP, PV, PP, MPU, IPU, S, SPAS, SREM, RSBN, SM, ZM, MAPA, ATRG, DTRG, _SIZE};
        const QStringList COL_STR = (QStringList() << "N" << "Identifier" << "Type" << "Altitude" << "Latitude" << "Longitude" << "MD" << "OZMPUv" << "OZMPUp" << "Pv" << "Pp"
                                                   << "MPU" << "IPU" << "S" << "Spas" << "Srem" << "RSBN and/or VORDME" << "Sm" << "Zm" << "Map Angle" << "A targ" << "D targ");
        int dat;

        struct
        {
            double speed = 0.80;
            double vs = 8.0;
            double twc = 0.0;
            double fl = 0;
            double isa = 0.0;
        }fplData;

        QLabel* qFork = NULL;
        QLabel* qTOD = NULL;
        double dTOD = -1;
        NVUPOINT* pTOD = NULL;

        //_KM(km) _FL(km), _MACH(mach or km/h), _VS(m/s), _TWC(km/h), _ISA(celsius)
        NVUPOINT* calculateTOD(NVUPOINT *&_pc, double& _KM, double _FL, double _MACH, double _VS, double _TWC, double _ISA);
        void deleteWaypoint(int row);                                   //Delete waypoint at row
        void insertRoute(std::vector<NVUPOINT*>, int row);  //Insert a list of waypoints, before, after or replace and then insert after a row.
        void insertWaypoint(NVUPOINT* wp, int row);                     //Insert a waypoint before the row.
        void replaceWaypoint(NVUPOINT* wp, int row);                    //Replace waypoint at row
        void moveWaypoint(int row, bool up);                            //Move waypoint at row up or down
        void refreshFlightplan();                                       //Refresh all waypoints
        void clearFlightplan();                                         //Delete all waypoints and clear the flightplan
        void autoGenerateCorrectionBeacons();
        const std::vector<NVUPOINT *> &getWaypoints();
        NVUPOINT* getWaypoint(int row, bool lastOutOfIndex = false);    //Returns NULL if out of index (if lastOutOfIndex is set to true, it instead returns last waypoint, if any exist)
        void selectNone();
        void mousePressEvent(QMouseEvent* event);
        QFlightplanTable(QWidget*&w);
};

#endif // QFLIGHTPLANTABLE_H
