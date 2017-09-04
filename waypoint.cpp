#include "waypoint.h"
#include <airway.h>
#include <airport_data.h>

/*
const int WAYPOINT::ORIGIN_AIRAC_AIRPORTS = 1;
const int WAYPOINT::ORIGIN_AIRAC_NAVAIDS = 2;
const int WAYPOINT::ORIGIN_AIRAC_WAYPOINTS = 3;
const int WAYPOINT::ORIGIN_FMS = 4;
const int WAYPOINT::ORIGIN_EARTHNAV = 5;
const int WAYPOINT::ORIGIN_RSBN = 6;
const int WAYPOINT::ORIGIN_XNVU_FLIGHTPLAN = 7;
const int WAYPOINT::ORIGIN_XNVU = 8;
const int WAYPOINT::ORIGIN_AIRAC_ATS = 9;
const int WAYPOINT::ORIGIN_XNVU_TEMP = 10;

const int WAYPOINT::TYPE_AIRPORT = 1;
const int WAYPOINT::TYPE_NDB = 2;
const int WAYPOINT::TYPE_VORDME = 3;
const int WAYPOINT::TYPE_VOR = 4;
const int WAYPOINT::TYPE_DME = 5;
const int WAYPOINT::TYPE_RSBN = 6;
const int WAYPOINT::TYPE_FIX = 7;
const int WAYPOINT::TYPE_LATLON = 8;
const int WAYPOINT::TYPE_AIRWAY = 9;
*/

/*
bool WAYPOINT::compare(WAYPOINT *p)
{
    if(latlon.x!=p->latlon.x) return false;
    if(latlon.y!=p->latlon.y) return false;
    if(name.compare(p->name)!=0) return false;
    if(name2.compare(p->name2)!=0) return false;
    if(range!=p->range) return false;
    if(freq!=p->freq) return false;
    if(alt!=p->alt) return false;
    if(elev!=p->elev) return false;
    if(trans_alt!=p->trans_alt) return false;
    if(trans_level!=p->trans_level) return false;
    if(length!=p->lent_runway) return false;
    if(country.compare(p->country)!=0) return false;
    if(ADEV!=p->ADEV) return false;

    return true;
}
*/

void WAYPOINT::swap(WAYPOINT& m, const WAYPOINT& wp)
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
    m.length = wp.length;
    m.width = wp.width;
    m.surface = wp.surface;
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

WAYPOINT::WAYPOINT()
{
    type = -1;
    data = NULL;
}

WAYPOINT::WAYPOINT(const WAYPOINT& wp)
{
    swap(*this, wp);
}
WAYPOINT& WAYPOINT::operator=(const WAYPOINT& other)
{
    swap(*this, other);
    return *this;
}

WAYPOINT* WAYPOINT::clone()
{
    return new WAYPOINT(*this);
}

WAYPOINT::~WAYPOINT()
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

QString WAYPOINT::lonToStr(double y)
{
    double l1, l2;
    l1 = fabs(modf(y, &l2)*60.0);
    int i2 = (int) fabs(l2);
    return (y < 0 ? "W" : "E") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2);
}

QString WAYPOINT::latToStr(double x)
{
    double l1, l2;
    l1 = fabs(modf(x, &l2)*60.0);
    int i2 = (int) fabs(l2);
    return (x < 0 ? "S" : "N") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2);
}

QString WAYPOINT::getTypeStr(const WAYPOINT* wp, int _type)
{
    int t = (wp) ? wp->type : _type;
    if(t == TYPE_AIRPORT) return "AIRPORT";
    if(t == TYPE_NDB) return "NDB";
    if(t == TYPE_VOR) return "VOR";
    if(t == TYPE_VORDME) return "VOR/DME";
    if(t == TYPE_DME) return "DME";
    if(t == TYPE_RSBN) return "RSBN";
    if(t == TYPE_FIX) return "FIX";
    if(t == TYPE_LATLON) return "LAT/LON";
    if(t == TYPE_AIRWAY) return "AIRWAY";
    if(t == TYPE_ILS) return "ILS";
    if(t == TYPE_LOC) return "LOC";
    if(t == TYPE_VORTAC) return "VORTAC";
    if(t == TYPE_TACAN) return "TACAN";
    if(t == TYPE_VHFNAV) return "VHF NAV";
    if(t == TYPE_RUNWAY) return "RUNWAY";
    if(t == TYPE_HELIPAD) return "HELIPAD";
    //if(t == TYPE_STAR) return "STAR";
    //if(t == TYPE_SID) return "SID";
    //if(t == TYPE_APPTR) return "TRANSITION";
    //if(t == TYPE_FINAL) return "FINAL";

    return "???";
}

QString WAYPOINT::getRunwaySurfaceStr(const WAYPOINT* wp, int _surface)
{
    int t = (wp) ? wp->surface : _surface;

    if(t == SURFACE_ASPHALT) return "Asphalt";
    if(t == SURFACE_CONCRETE) return "Concrete";
    if(t == SURFACE_DIRT) return "Dirt";
    if(t == SURFACE_DRY_LAKEBED) return "Dry lakebed";
    if(t == SURFACE_GRAVEL) return "Gravel";
    if(t == SURFACE_SNOW_ICE) return "Snow/Ice";
    if(t == SURFACE_TURF) return "Turf";
    if(t == SURFACE_WATER) return "Water";

    return "Unknown";
}

/*
std::vector<QString> WAYPOINT::getTypeStrList()
{
    //TODO: Bad coding, very bad coding. Need to reprogram.
    //WARNING: THESE NEED TO BE IN ORDER!
    std::vector<QString> rTStr;
    rTStr.push_back(getTypeStr(NULL, TYPE_AIRPORT));
    rTStr.push_back(getTypeStr(NULL, TYPE_NDB));
    rTStr.push_back(getTypeStr(NULL, TYPE_VORDME));
    rTStr.push_back(getTypeStr(NULL, TYPE_VOR));
    rTStr.push_back(getTypeStr(NULL, TYPE_DME));
    rTStr.push_back(getTypeStr(NULL, TYPE_RSBN));
    rTStr.push_back(getTypeStr(NULL, TYPE_FIX));
    rTStr.push_back(getTypeStr(NULL, TYPE_LATLON));
    rTStr.push_back(getTypeStr(NULL, TYPE_AIRWAY));
    rTStr.push_back(getTypeStr(NULL, TYPE_ILS));
    rTStr.push_back(getTypeStr(NULL, TYPE_TACAN));
    rTStr.push_back(getTypeStr(NULL, TYPE_VORTAC));
    rTStr.push_back(getTypeStr(NULL, TYPE_VHFNAV));
    rTStr.push_back(getTypeStr(NULL, TYPE_RUNWAY));
    rTStr.push_back(getTypeStr(NULL, TYPE_HELIPAD));

    //rTStr.push_back(getTypeStr(NULL, TYPE_STAR));
    //rTStr.push_back(getTypeStr(NULL, TYPE_SID));
    //rTStr.push_back(getTypeStr(NULL, TYPE_APPTR));
    //rTStr.push_back(getTypeStr(NULL, TYPE_FINAL));
    return rTStr;
}
*/

QString WAYPOINT::getOriginStr(int _origin)
{
    switch(_origin)
    {
        case ORIGIN_FMS: return "Imported from user FMS flightplan";
        case ORIGIN_XWP: return "Imported from user XWP flightplan";
        case ORIGIN_FLIGHTPLAN: return  "Custom flightplan waypoint";

        case ORIGIN_RSBN: return "rsbn.dat (RSBN library)";
        case ORIGIN_XNVU: return "xnvu_wps.txt (XNVU library)";

        case ORIGIN_X10_AIRAC_AIRPORTS: return "airports.txt (GNS430 AIRAC)";
        case ORIGIN_X10_AIRAC_NAVAIDS: return "navaids.txt (GNS430 AIRAC)";
        case ORIGIN_X10_AIRAC_FIXES: return "waypoints.txt (GNS430 AIRAC)";
        case ORIGIN_X10_AIRAC_ATS: return "ats.txt (GNS430 AIRAC)";
        case ORIGIN_X10_EARTHNAV: return "earth_nav.dat";

        case ORIGIN_X11_CUSTOM_AIRPORTS: return "Custom apt.dat";
        case ORIGIN_X11_DEFAULT_AIRPORTS: return "Default apt.dat";
        case ORIGIN_X11_GATEWAY_AIRPORTS: return "Gateway apt.dat";
        case ORIGIN_X11_CUSTOM_EARTHNAV: return "Custom earth_nav.dat";
        case ORIGIN_X11_DEFAULT_EARTHNAV: return "Default earth_nav.dat";
        case ORIGIN_X11_CUSTOM_ATS: return "Custom earth_awy.dat";
        case ORIGIN_X11_DEFAULT_ATS: return "Default earth_awy.dat";
        case ORIGIN_X11_CUSTOM_FIXES: return "Custom earth_fix.dat";
        case ORIGIN_X11_DEFAULT_FIXES: return "Default earth_fix.dat";
    }

    return "Unknown";
}



bool WAYPOINT::isNavaid(int _type)
{
    if(_type == TYPE_NDB) return true;
    if(_type == TYPE_VORDME) return true;
    if(_type == TYPE_VOR) return true;
    if(_type == TYPE_ILS) return true;
    if(_type == TYPE_LOC) return true;
    if(_type == TYPE_DME) return true;
    if(_type == TYPE_RSBN) return true;
    if(_type == TYPE_VORTAC) return true;
    if(_type == TYPE_TACAN) return true;
    if(_type == TYPE_VHFNAV) return true;

    return false;
}
