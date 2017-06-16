#ifndef WAYPOINT_HEADER
#define WAYPOINT_HEADER

#include <QString>
#include <CPoint.h>
#include <cmath>
#include <vector>
#include <LMATH.h>

//AIRPORT DATA definitions
#define DEFAULT_WAYPOINT_MARGIN 0.0001 //1 meter
#define SURFACE_ASPHALT		1	//(HRD)
#define SURFACE_CONCRETE 	2	//(HRD)
#define SURACE_TURF			3	//GRASS	(TRF)
#define SURFACE_DIRT		4	//(DRT)
#define SURFACE_GRAVEL		5	//(GRV)
#define SURFACE_DRY_LAKEBED 12	//(SND)
#define SURFACE_WATER		13
#define SURFACE_SNOW_ICE	14	//(SNW)
#define SURFACE_UNKOWN		15	//Transparent, custom airports
#define ATC_ATIS	50
#define ATC_UNICOM	51
#define ATC_DEL		52
#define ATC_GND		53
#define ATC_TWR		54
#define ATC_APP		55
#define ATC_DEP		56


class WAYPOINT;

class AIRWAY
{
    private:
        static void swap(AIRWAY& a, const AIRWAY& r)
        {
            a.name = r.name;
            a.distance = r.distance;
            a.lATS.clear();
            for(unsigned int i=0; i<r.lATS.size(); i++) a.lATS.push_back(r.lATS[i]);
        }

    public:
        QString name;
        double distance;
        std::vector<WAYPOINT*> lATS;    //Waypoints are not deleted when removing this airway, as they already are contained and manager in the lWP list.

        AIRWAY()
        {
            name = "";
            distance = -1;
        }

        AIRWAY(const AIRWAY& awy)
        {
            swap(*this, awy);
        }
        AIRWAY& operator=(const AIRWAY& other)
        {
            swap(*this, other);
            return *this;
        }

        AIRWAY* clone()
        {
            return new AIRWAY(*this);
        }
};
class RUNWAY
{
    private:
        static void swap(RUNWAY& me, const RUNWAY& r)
        {
            me.rwyA = r.rwyA;
            me.rwyB = r.rwyB;
            me.lightA = r.lightA;
            me.lightB = r.lightB;
            me.surface = r.surface;
            me.start = r.start;
            me.end = r.end;
        }

    public:
        QString rwyA;   //Runway identifer
        QString rwyB;   //Runway opposite identifer
        int lightA;		//Runway approach lightning
        int lightB;		//Runway approach lightning opposite
        int surface;	//Runway surface
        CPoint start;   //Coordinates of runway start threshold
        CPoint end;     //Coordinates of runway end threshold

        RUNWAY(const QString& _rwyA, const CPoint& _startA, int _lightA, const QString _rwyB, const CPoint& _startB, int _lightB, int _surface)
        {
            rwyA = _rwyA;
            start = _startA;
            lightA = _lightA;
            rwyB = _rwyB;
            end = _startB;
            lightB = _lightB;
            surface = _surface;
        }

        RUNWAY(const RUNWAY& r)
        {
            swap(*this, r);
        }

        RUNWAY& operator=(const RUNWAY& other)
        {
            swap(*this, other);
            return *this;
        }

        RUNWAY* clone()
        {
            return new RUNWAY(*this);
        }

        int getLength()	//In feet
        {
            return LMATH::meterToFeet(LMATH::calc_distance(start, end)*1000);
        }

        ~RUNWAY()
        {

        }
};
class AIRPORT_DATA
{
    private:
        static void swap(AIRPORT_DATA& a, const AIRPORT_DATA& r)
        {
            for(unsigned int i=0; i<a.lRunways.size(); i++) delete a.lRunways[i];
            a.lRunways.clear();
            a.lFreq.clear();

            a.city = r.city;
            for(unsigned int i=0; i<r.lRunways.size(); i++) a.lRunways.push_back(new RUNWAY(*r.lRunways[i]));
            for(unsigned int i=0; i<r.lFreq.size(); i++) a.lFreq.push_back(std::make_pair(r.lFreq[i].first, r.lFreq[i].second));
        }

    public:
        QString city;                               //The city airport belongs to
        std::vector<RUNWAY*> lRunways;              //List of runways belonging to airport
        std::vector<std::pair<int, int> > lFreq;

        AIRPORT_DATA()
        {
            city = "";
        }

        AIRPORT_DATA(const AIRPORT_DATA& ad)
        {
            swap(*this, ad);
        }

        AIRPORT_DATA& operator=(const AIRPORT_DATA& other)
        {
            swap(*this, other);
            return *this;
        }

        AIRPORT_DATA* clone()
        {
            return new AIRPORT_DATA(*this);
        }

        ~AIRPORT_DATA()
        {
            for(unsigned int i=0; i<lRunways.size(); i++) delete lRunways[i];
        }
};

class WAYPOINT
{
    private:
        static void swap(WAYPOINT& m, const WAYPOINT& wp)
        {
            if(m.type == TYPE_AIRWAY && m.data!=NULL)
            {
                AIRWAY* awy = (AIRWAY*) m.data;
                delete awy;
            }//if
            else if(m.type == TYPE_AIRPORT && m.data!=NULL)
            {
                AIRPORT_DATA* aRwys = (AIRPORT_DATA*) m.data;
                delete aRwys;
            }//if

            m.latlon = wp.latlon;
            m.type = wp.type;
            m.name = wp.name;
            m.name2 = wp.name2;
            m.range = wp.range;
            m.freq = wp.freq;
            m.alt = wp.alt;
            m.elev = wp.elev;
            m.trans_alt = wp.trans_alt;
            m.trans_level = wp.trans_level;
            m.longest_runway = wp.longest_runway;
            m.country = wp.country;
            m.MD = wp.MD;
            m.ADEV = wp.ADEV;          //Angle deviation of VOR/DME from X-Plane earth_nav.dat
            m.wpOrigin = wp.wpOrigin;         //If waypoint has not been converted from FMS (1), is retrieved from earth_nav.dat (2), or is custom made (3)
            m.data = NULL;

            if(wp.type == TYPE_AIRPORT && wp.data!=NULL)
            {
                AIRPORT_DATA* apd = (AIRPORT_DATA*) wp.data;
                m.data = (void*) new AIRPORT_DATA(*apd);
            }
            else if(wp.type == TYPE_AIRWAY && wp.data!=NULL)
            {
                AIRWAY* awy = (AIRWAY*) wp.data;
                m.data = (void*) new AIRWAY(*awy);
            }
        }

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
        void* data = NULL;

        bool compare(WAYPOINT*);

        static QString lonToStr(double y);
        static QString latToStr(double x);
        static QString getTypeStr(const WAYPOINT* wp, int _type = 0);
        static std::vector<QString> getTypeStrList();
        static bool isNavaid(int _type);


        WAYPOINT()
        {
            type = -1;
            data = NULL;
        }

        WAYPOINT(const WAYPOINT& wp)
        {
            swap(*this, wp);
        }
        WAYPOINT& operator=(const WAYPOINT& other)
        {
            swap(*this, other);
            return *this;
        }

        WAYPOINT* clone()
        {
            return new WAYPOINT(*this);
        }

        virtual ~WAYPOINT()
        {
            if(type == TYPE_AIRWAY && data!=NULL)
            {
                AIRWAY* awy = (AIRWAY*) data;
                delete awy;
            }//if
            else if(type == TYPE_AIRPORT && data!=NULL)
            {
                AIRPORT_DATA* aRwys = (AIRPORT_DATA*) data;
                delete aRwys;
            }//if
        }
};




/*

//SIDS identifers
#define PROC_TYPE_ENGOUT_SID            0       //RWYxxy -> col4
#define PROC_TYPE_SID_RWY_TRANS         1       //RWYxxy -> col4
#define PROC_TYPE_SID                   2       //If there is no 1, 4 or F in any SID identifiers same as this one
#define PROC_TYPE_SID_ENR_TRANS         3       //Transition identifer -> col4
#define PROC_TYPE_RNAV_SID_RWY_TRANS    4       //RWYxxy -> col4
#define PROC_TYPE_RNAV_SID              5
#define PROC_TYPE_RNAV_SID_ENR_TRANS    6       //Transition identifier ->col4
#define PROC_TYPE_FMS_SID_RWY_TRANS     'F'     //RWYxxy -> col4
#define PROC_TYPE_FMS_SID               'M'
#define PROC_TYPE_FMS_SID_ENR_TRANS     'S'
#define PROC_TYPE_VECTOR_SID_RWY_TRANS  'T'     //RWYxxy -> col4
#define PROC_TYPE_VECTOR_SID_ENR_TRANS  'V'

//STARS identifiers
#define PROC_TYPE_STAR_ENR_TRANS        1       //Transition identifer -> col4
#define PROC_TYPE_STAR                  2
#define PROC_TYPE_STAR_RWY_TRANS        3       //RWYxxy -> col4 or blank
#define PROC_TYPE_RNAV_STAR_ENR_TRANS   4       //Transition identifer -> col4
#define PROC_TYPE_RNAV_STAR             5
#define PROC_TYPE_RNAV_RWY_TRANS        6       //RWYxxy -> col4 or blank
#define PROC_TYPE_PRF_DES_ENR_TRANS     7
#define PROC_TYPE_PRF_DES_COM_RTE       8
#define PROC_TYPE_PRF_DES_RWY_TRANS     9       //RWYxxy -> col4 or blank
#define PROC_TYPE_FMS_STAR_ENR_TRANS    'F'     //Transition identifer -> col4
#define PROC_TYPE_FMS_STAR              'M'
#define PROC_TYPE_FMS_STAR_RWY_TRANS    'S'

//APPR first column
#define PROC_TYPE_APPR_TRANS            'A'
#define PROC_TYPE_APPR_BCRS             'B'
#define PROC_TYPE_APPR_GPS_REQ          'E'
#define PROC_TYPE_APPR_FMS_GUI          'F'
#define PROC_TYPE_APPR_IGS              'G'
#define PROC_TYPE_APPR_ILS              'I'
#define PROC_TYPE_APPR_GLS              'J'
#define PROC_TYPE_APPR_WAAS             'K'
#define PROC_TYPE_APPR_LOC_ONLY         'L'
#define PROC_TYPE_APPR_MLS              'M'
#define PROC_TYPE_APPR_NDB              'N'
#define PROC_TYPE_APPR_GPS              'P'
#define PROC_TYPE_APPR_RNAV             'R'
#define PROC_TYPE_APPR_TACAN            'T'
#define PROC_TYPE_APPR_SDF              'U'
#define PROC_TYPE_APPR_VOR              'V'
#define PROC_TYPE_APPR_MLS_A            'W'
#define PROC_TYPE_APPR_LDA              'X'
#define PROC_TYPE_APPR_MLS_B            'Y'
#define PROC_TYPE_APPR_MISSED           'Z'

//APPR second column
#define PROC_TYPE_APPR_DME_REQ          'D'
#define PROC_TYPE_APPR_DME_NOT_REQ      'N'
#define PROC_TYPE_APPR_DMEDME_REQ       'T'
#define PROC_TYPE_APPR_PRI_MISSED       'P'
#define PROC_TYPE_APPR_SEC_MISSED       'S'

//APPR third column
#define PROC_TYPE_APPR_CTL_MIN          'C'
#define PROC_TYPE_APPR_SI_MIN           'S'
#define PROC_TYPE_APPR_HELI             'H'

class PROC  //SID, STAR, APPR
{
    public:
        //int
        QString routeType; //See above definitions for SID, STAR and APPR
        WAYPOINT* airport;  //This is the airport the sid/star/appr belongs to.
};

*/

#endif
