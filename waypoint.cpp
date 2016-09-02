#include "waypoint.h"

const int WAYPOINT::ORIGIN_AIRAC_AIRPORTS = 1;
const int WAYPOINT::ORIGIN_AIRAC_NAVAIDS = 2;
const int WAYPOINT::ORIGIN_AIRAC_WAYPOINTS = 3;
const int WAYPOINT::ORIGIN_AIRAC_ATS = 9;
const int WAYPOINT::ORIGIN_FMS = 4;
const int WAYPOINT::ORIGIN_EARTHNAV = 5;
const int WAYPOINT::ORIGIN_RSBN = 6;
const int WAYPOINT::ORIGIN_XNVU_FLIGHTPLAN = 7;
const int WAYPOINT::ORIGIN_XNVU = 8;

const int WAYPOINT::TYPE_AIRPORT = 1;
const int WAYPOINT::TYPE_NDB = 2;
const int WAYPOINT::TYPE_VORDME = 3;
const int WAYPOINT::TYPE_VOR = 4;
const int WAYPOINT::TYPE_DME = 5;
const int WAYPOINT::TYPE_RSBN = 6;
const int WAYPOINT::TYPE_FIX = 7;
const int WAYPOINT::TYPE_LATLON = 8;
const int WAYPOINT::TYPE_AIRWAY = 9;

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
    if(longest_runway!=p->longest_runway) return false;
    if(country.compare(p->country)!=0) return false;
    if(ADEV!=p->ADEV) return false;

    return true;
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
    if(t == TYPE_AIRPORT) return "Airport";
    if(t == TYPE_NDB) return "NDB";
    if(t == TYPE_VOR) return "VOR";
    if(t == TYPE_VORDME) return "VOR/DME";
    if(t == TYPE_DME) return "DME";
    if(t == TYPE_RSBN) return "RSBN";
    if(t == TYPE_FIX) return "FIX";
    if(t == TYPE_LATLON) return "LAT/LON";
    if(t == TYPE_AIRWAY) return "AIRWAY";

    return "NAN";
};

std::vector<QString> WAYPOINT::getTypeStrList()
{

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

    return rTStr;
};

bool WAYPOINT::isNavaid(int _type)
{
    if(_type == TYPE_NDB) return true;
    if(_type == TYPE_VORDME) return true;
    if(_type == TYPE_VOR) return true;
    if(_type == TYPE_DME) return true;
    if(_type == TYPE_RSBN) return true;

    return false;
}
