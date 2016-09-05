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
        static std::multimap<QString, NVUPOINT*> lWP;
        static std::multimap<QString, NVUPOINT*> lWP2;
        static std::vector<NVUPOINT*> lXNVURemove;
        static void validate_airport(const QStringList& record);
        static void validate_navaid(const QStringList& record);
        static void validate_waypoint(const QStringList &record);
        static void validate_airways(QFile&);
        static void validate_earthnav(const QStringList& record);
        static void validate_RSBN(const QStringList& record);
        static void validate_xnvu(const QStringList& record);
        static void validate_fms(const QStringList& record);
        static void validate_xnvuflightplan(const QStringList& record);


    public:
        static int dat;

        //Loaded from i.e. navigraph (airports.txt, navaids.txt, waypoints.txt, ats.txt)
        static std::vector<NVUPOINT*> lAirports;
        static std::vector<NVUPOINT*> lNDB;
        static std::vector<NVUPOINT*> lVOR;
        static std::vector<NVUPOINT*> lDME;
        static std::vector<NVUPOINT*> lVORDME;         //Cannot be used as RSBN, as there is no angle deviation data.
        static std::vector<NVUPOINT*> lFixes;
        static std::vector<AIRWAY*> lAirways;

        //Loaded from earth_nav.txt
        static std::vector<NVUPOINT*> lXNDB;
        static std::vector<NVUPOINT*> lXVOR;
        static std::vector<NVUPOINT*> lXDME;
        static std::vector<NVUPOINT*> lXVORDME;        //Loaded from X-Plane earth earth_nav.dat, as it includes VOR angle deviation data.

        //Loaded from rsbn.dat
        static std::vector<NVUPOINT*> lRSBN;

        //Loaded from XNVU local library xnvu.dat
        static std::vector<NVUPOINT*> lXNVU;

        //Tempory created waypoints for this instance (not added to search function, hence lWP and lWP2 list).
        static std::vector<NVUPOINT*> lXNVUTemp;

        //Loaded from user *.fms
        static std::vector<NVUPOINT*> lFMS;

        //loaded from user *.wps
        static std::vector<NVUPOINT*> lXNVUFlightplan;

        static NVUPOINT* validate_custom_point(const NVUPOINT* wpRef, const QString& record);

        static QString getAirwayWaypointsBetween(QString& airway, NVUPOINT* wpA, NVUPOINT* wpB, std::vector<NVUPOINT *> &lA, bool allowOpposite);
        static QString getRoute(const QString& _qstr, std::vector<NVUPOINT*>& route);
        static std::vector<NVUPOINT*> search(const QString& _name);
        static std::vector< std::pair<NVUPOINT*, double> > getClosestRSBN(const NVUPOINT* wp, int n, double d, bool includeVOR);
        static NVUPOINT* getClosestSimilarWaypoint(NVUPOINT* wp, double &distance);

        static void addXNVUWaypoint(NVUPOINT* lP);
        static void addXNVUWaypointTempory(NVUPOINT* lP);
        static void addXNVUData(std::vector<NVUPOINT*> lP);
        static QString load(int dat);
        static int _load(const QString& file, int _type);
        static int saveXNVUData();
        static void removeWPSPoints(const std::vector<NVUPOINT *> &pR);

        static int loadFMS(const QString& file);
        static int saveFMS(const QString& file, std::vector<NVUPOINT*> lN);
        static void removeFMS();
        static int FMSToXNVUType(int _type);
        static int XNVUToFMSType(int _type);

        static int loadXNVUFlightplan(const QString& file);
        static int saveXNVUFlightplan(const QString& file, std::vector<NVUPOINT*> lN);
        static void removeXNVUFlightplan();

        static void clear();

        XFMS_DATA();
        ~XFMS_DATA();
};

#endif

