#ifndef WAYPOINT_HEADER
#define WAYPOINT_HEADER

#include <QString>
#include <CPoint.h>
#include <cmath>
#include <vector>

class WAYPOINT
{
	public:
        static constexpr int ORIGIN_AIRAC_AIRPORTS = 1;
        static constexpr int ORIGIN_AIRAC_NAVAIDS = 2;
        static constexpr int ORIGIN_AIRAC_WAYPOINTS = 3;
        static constexpr int ORIGIN_AIRAC_ATS = 4;
        static constexpr int ORIGIN_FMS = 5;
        static constexpr int ORIGIN_EARTHNAV = 6;
        static constexpr int ORIGIN_RSBN = 7;
        static constexpr int ORIGIN_XNVU_FLIGHTPLAN = 8;
        static constexpr int ORIGIN_XNVU = 9;
        static constexpr int ORIGIN_XNVU_TEMP = 10;

        static constexpr int TYPE_AIRPORT = 1;
        static constexpr int TYPE_NDB = 2;
        static constexpr int TYPE_VORDME = 3;
        static constexpr int TYPE_VOR = 4;
        static constexpr int TYPE_DME = 5;
        static constexpr int TYPE_RSBN = 6;
        static constexpr int TYPE_FIX = 7;
        static constexpr int TYPE_LATLON = 8;
        static constexpr int TYPE_AIRWAY = 9;


        CPoint latlon;
        int type = -1;
        QString name;
        QString name2;
        int range = 0;
		double freq = 0; 
        int alt = 0;
        int elev = 0;
        int trans_alt = 0;
        int trans_level = 0;
        int longest_runway = 0;
        QString country;
        double MD = 0;
        double ADEV = 0;          //Angle deviation of VOR/DME from X-Plane earth_nav.dat
        int wpOrigin = 0;         //If waypoint has not been converted from FMS (1), is retrieved from earth_nav.dat (2), or is custom made (3)
        void* data;

        bool compare(WAYPOINT*);

        static QString lonToStr(double y);
        static QString latToStr(double x);
        static QString getTypeStr(const WAYPOINT* wp, int _type = 0);
        static std::vector<QString> getTypeStrList();
        static bool isNavaid(int _type);


		WAYPOINT(){};
        virtual ~WAYPOINT(){};
};

class AIRWAY
{
    public:
        QString name;
        double distance;
        std::vector<WAYPOINT*> lATS;

        AIRWAY(){};
};

#endif
