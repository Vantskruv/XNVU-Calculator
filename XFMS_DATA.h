#ifndef XFMS_DATA_HEADER
#define XFMS_DATA_HEADER

#include <map>
#include <vector>
#include <QString>
#include <NVU.h>
#include <QFile>

#define XNVU_WPS_FILENAME "xnvu_wps.txt"

class XFMS_DATA
{
	private:
        static std::multimap<QString, NVUPOINT*> lWP;               //QString is IDENTIFIER of waypoint NVUPOINT
        static std::multimap<QString, NVUPOINT*> lWP2;              //QString is the NAME of waypoint NVUPOINT

        static void validate_airport(const QStringList& record);
        static void validate_navaid(const QStringList& record);
        static void validate_waypoint(const QStringList &record);
        static void validate_airways(QFile&);
        static void validate_earthnav(const QStringList& record);
        static void validate_RSBN(const QStringList& record);
        static void validate_xnvu(const QStringList& record);
        static void validate_fms(std::vector<NVUPOINT*>& lFMS, const QStringList& record);
        static void validate_xnvuflightplan(std::vector<NVUPOINT*>& lXNVUFlightplan, const QStringList& record);
        //Returns a new allocated waypoint if successful. If waypoint is a bearing/distance waypoint, wpRef needs to be set.
        //If error occurs, a description of error is returned.
        static QString validate_custom_point(const NVUPOINT* wpRef, NVUPOINT *&rPoint, const QString& record);

    public:
        static int dat;

        //Loaded from i.e. navigraph (airports.txt, navaids.txt, waypoints.txt, ats.txt)
        static std::vector<NVUPOINT*> lAirports;
        static std::vector<NVUPOINT*> lNDB;
        static std::vector<NVUPOINT*> lVOR;
        static std::vector<NVUPOINT*> lDME;
        static std::vector<NVUPOINT*> lVORDME;         //Cannot be used as RSBN, as there is no angle deviation data.
        static std::vector<NVUPOINT*> lILS;
        static std::vector<NVUPOINT*> lFixes;
        static std::vector<AIRWAY*> lAirways;

        //Loaded from earth_nav.txt
        static std::vector<NVUPOINT*> lXNDB;
        static std::vector<NVUPOINT*> lXVOR;
        static std::vector<NVUPOINT*> lXDME;
        static std::vector<NVUPOINT*> lXVORDME;        //Loaded from X-Plane earth earth_nav.dat, as it includes VOR angle deviation data.

        //Loaded from rsbn.dat
        static std::vector<NVUPOINT*> lRSBN;

        //Loaded from XNVU local library from file XNVU_WPS_FILENAME (see top of this file for filename)
        static std::vector<NVUPOINT*> lXNVU;


        //Get a list of waypoints between two waypoint in an airway
        /*
         * airway: Name of airway
         * wpA: Waypoint A
         * wpB: Waypoint B
         * lA: Returned list of waypoints (note that wpA and wpB is not included in list)
         * allowOpposite: Allow an opposite direction of airway if the correct direction of airway is not found.
         * Returns a string of description if error occurs.
        */
        static QString getAirwayWaypointsBetween(QString& airway, NVUPOINT* wpA, NVUPOINT* wpB, std::vector<NVUPOINT *> &lA, bool allowOpposite);

        //Create a route defined by a string. Arguments are:
        /* _qstr: route string,
         * route: returned route,
         * wpRef: If set, align first point in route to the nearest of wpRef
        */
        static QString getRoute(const QString& _qstr, std::vector<NVUPOINT*>& route, NVUPOINT *wpRef = NULL);

        //Returns a list of found waypoints. Searches for ID and NAME of waypoint.
        static std::vector<NVUPOINT*> search(const QString& _name);

        //Returns a list pairs containing waypoint and distance to wp
        /*
         * wp: waypoint of origo
         * n: maximum number of waypoints to return
         * d: maximum distance allowed
         * includeVOR: Set to TRUE if you want to include VOR:s aswell.
        */
        static std::vector< std::pair<NVUPOINT*, double> > getClosestRSBN(const NVUPOINT* wp, int n, double d, bool includeVOR);

        //Returns closest waypoint with same type and name and its distance. If not found, returns NULL.
        static NVUPOINT* getClosestWaypointType(NVUPOINT* wp, double &distance);

        static void addXNVUWaypoint(NVUPOINT* lP);
        static void addXNVUWaypointTempory(NVUPOINT* lP);
        static void addXNVUData(std::vector<NVUPOINT*> lP);
        static QString load(int dat);
        static int _load(const QString& file, int _type);
        static int saveXNVUData();

        static std::vector<NVUPOINT*> loadFMS(const QString& file);
        static int saveFMS(const QString& file, const std::vector<NVUPOINT *> lN);
        static void removeFMS();
        static int FMSToXNVUType(int _type);
        static int XNVUToFMSType(int _type);

        static std::vector<NVUPOINT*> loadXNVUFlightplan(const QString& file);
        static int saveXNVUFlightplan(const QString& file, std::vector<NVUPOINT*> lN);
        static void removeXNVUFlightplan();

        static void setDate(int dat);
        static void clear();

        XFMS_DATA();
        ~XFMS_DATA();
};

#endif

