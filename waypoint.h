#ifndef WAYPOINT_HEADER
#define WAYPOINT_HEADER

#include <QString>
#include <CPoint.h>
#include <cmath>
#include <vector>

#define DEFAULT_WAYPOINT_MARGIN 0.0001 //1 meter

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
        static constexpr int ORIGIN_WPS = 8;
        static constexpr int ORIGIN_XNVU = 9;
        static constexpr int ORIGIN_FLIGHTPLAN = 10;
        static constexpr int ORIGIN_X11_CUSTOM_AIRPORTS = 11;
        static constexpr int ORIGIN_X11_DEFAULT_AIRPORTS = 12;
        static constexpr int ORIGIN_X11_GATEWAY_AIRPORTS = 13;

        static constexpr int TYPE_AIRPORT = 1;
        static constexpr int TYPE_NDB = 2;
        static constexpr int TYPE_VORDME = 3;
        static constexpr int TYPE_VOR = 4;
        static constexpr int TYPE_DME = 5;
        static constexpr int TYPE_RSBN = 6;
        static constexpr int TYPE_FIX = 7;
        static constexpr int TYPE_LATLON = 8;
        static constexpr int TYPE_AIRWAY = 9;
        static constexpr int TYPE_ILS = 10;
        static constexpr int TYPE_TACAN = 11;
        static constexpr int TYPE_VORTAC = 12;
        static constexpr int TYPE_VHFNAV = 13; //Includes VORDME/VOR/DME/VORTAC/TACAN
        //static constexpr int TYPE_STAR = 13;
        //static constexpr int TYPE_SID = 14;
        //static constexpr int TYPE_APPTR = 15;
        //static constexpr int TYPE_FINAL = 16;


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
