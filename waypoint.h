#ifndef WAYPOINT_HEADER
#define WAYPOINT_HEADER

#include <QString>
#include <CPoint.h>
#include <cmath>
#include <vector>

class WAYPOINT
{
	public:
        static const int ORIGIN_AIRAC_AIRPORTS;
        static const int ORIGIN_AIRAC_NAVAIDS;
        static const int ORIGIN_AIRAC_WAYPOINTS;
        static const int ORIGIN_FMS;
        static const int ORIGIN_EARTHNAV;
        static const int ORIGIN_RSBN;
        static const int ORIGIN_XNVU_FLIGHTPLAN;
        static const int ORIGIN_XNVU;

        static const int TYPE_AIRPORT;
        static const int TYPE_NDB;
        static const int TYPE_VORDME;
        static const int TYPE_VOR;
        static const int TYPE_DME;
        static const int TYPE_RSBN;
        static const int TYPE_FIX;
        static const int TYPE_LATLON;


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

        bool compare(WAYPOINT*);

        static QString lonToStr(double y);
        static QString latToStr(double x);
        static QString getTypeStr(const WAYPOINT* wp, int _type = 0);
        static std::vector<QString> getTypeStrList();
        static bool isNavaid(int _type);


		WAYPOINT(){};
        virtual ~WAYPOINT(){};
};

#endif
