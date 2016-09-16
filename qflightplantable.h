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
        //const QStringList COL_STR  = (QStringList() << "N" << "ID" <<  "TYPE" <<  "ALT" <<  "LAT" <<  "LON" <<  "MD" <<  "OZMPUV" <<  "OZMPUP" <<  "PV" <<  "PP" <<  "MPU" <<  "IPU" <<  "S" <<  "SPAS" <<  "SREM" <<  "RSBN" <<  "SM" <<  "ZM" <<  "MAPA" <<  "ATRG" <<  "DTRG");
        const QStringList COL_STR = (QStringList() << "N" << "Identifier" << "Type" << "Altitude" << "Latitude" << "Longitude" << "MD" << "OZMPUv" << "OZMPUp" << "Pv" << "Pp"
                                                   << "MPU" << "IPU" << "S" << "Spas" << "Srem" << "RSBN and/or VORDME" << "Sm" << "Zm" << "Map Angle" << "A targ" << "D targ");
/*
        static constexpr int COL_N = 0;
        static constexpr int COL_ID = 1;
        static constexpr int COL_TYPE = 2;
        static constexpr int COL_ALT = 3;
        static constexpr int COL_LAT = 4;
        static constexpr int COL_LON = 5;
        static constexpr int COL_MD;
        static constexpr int COL_S = 6;
        static constexpr int COL_SPAS = 7;
        static constexpr int COL_SREM = 8;
        static constexpr int COL_RSBN = 9;
        static constexpr int COL_OZMPUV = 10;
        static constexpr int COL_OZMPUP = 11;
        static constexpr int COL_PV = 12;
        static constexpr int COL_PP = 13;
        static constexpr int COL_MPU = 14;
        static constexpr int COL_IPU = 15;
        static constexpr int COL_ZM = 16;
        static constexpr int COL_SM = 17;
        static constexpr int COL_MAPA = 18;
        static constexpr int COL_ATRG = 19;
        static constexpr int COL_DTRG = 20;
*/
        int dat;
        QLabel* qFork = NULL;
        bool showFeet = false;

        void deleteWaypoint(int row);                                   //Delete waypoint at row
        //void insertWaypoint(NVUPOINT* wp, int row, int offset);         //Insert a waypoint with an offset value set. Before row (-1), after row (1), or replace the row (0).
        void insertRoute(std::vector<NVUPOINT*>, int row);  //Insert a list of waypoints, before, after or replace and then insert after a row.
        void insertWaypoint(NVUPOINT* wp, int row);                     //Insert a waypoint before the row.
        void replaceWaypoint(NVUPOINT* wp, int row);                    //Replace waypoint at row
        void moveWaypoint(int row, bool up);                            //Move waypoint at row up or down
        void refreshFlightplan();                                       //Refresh all waypoints
        void clearFlightplan();                                         //Delete all waypoints and clear the flightplan
        const std::vector<NVUPOINT *> &getWaypoints();
        NVUPOINT* getWaypoint(int row);

        void updateShownColumns();
        void mousePressEvent(QMouseEvent* event);
        QFlightplanTable(QWidget*&w);
};

#endif // QFLIGHTPLANTABLE_H
