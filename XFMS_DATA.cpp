#include "XFMS_DATA.h"
#include <airway.h>
#include <airport_data.h>

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <iostream>

#include <list>
#include <QString>
#include <QFile>
#include <QDir>
#include <QFileInfoList>
#include <QTextStream>
#include "LMATH.h"
#include "coremag.h"
#include <algorithm>
#include "dialogsettings.h"
#include <QDebug>

class RUNWAY
{
    private:
        static void swap(RUNWAY& me, const RUNWAY& r)
        {
            me.type = r.type;
            me.length = r.length;
            me.width = r.width;
            me.rwyA = r.rwyA;
            me.rwyB = r.rwyB;
            me.lightA = r.lightA;
            me.lightB = r.lightB;
            me.surface = r.surface;
            me.start = r.start;
            me.end = r.end;
        }

    public:
        int type;       //WAYPOINT::TYPE_RUNWAY or WAYPOINT::TYPE_HELIPAD
        double length;  //Length of runway/helipad
        double width;   //Width of runway/helipad
        QString rwyA;   //Runway identifer
        QString rwyB;   //Runway opposite identifer
        int lightA;		//Runway approach lightning
        int lightB;		//Runway approach lightning opposite
        int surface;	//Runway surface
        CPoint start;   //Coordinates of runway start threshold
        CPoint end;     //Coordinates of runway end threshold

        RUNWAY(const QString& _rwyA, const CPoint& _startA, int _lightA, const QString _rwyB, const CPoint& _startB, int _lightB, double _rwyWidth, int _surface, int _type)
        {
            type = _type;
            rwyA = _rwyA;
            start = _startA;
            lightA = _lightA;
            rwyB = _rwyB;
            end = _startB;
            lightB = _lightB;
            surface = _surface;
            width = _rwyWidth;
            length = LMATH::calc_distance(start, end)*1000;
        }

        RUNWAY(const QString & _rwy, const CPoint& _startA, double _length, double _width, int _surface, int _type)
        {
            type = _type;
            rwyA = _rwy;
            start = _startA;
            length = _length;
            width = _width;
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

        double getLength()	//In meter
        {
            return length;
        }

        double getWidth() { return width; }

        CPoint getMiddlePoint()
        {
            if(type == WAYPOINT::TYPE_RUNWAY) return (start + (end - start)*0.5);
            return start;
        }

        int getType(){ return type; }

        ~RUNWAY()
        {

        }
};


volatile int XFMS_DATA::__DATA_LOADED[8] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile int XFMS_DATA::__CURRENT_LOADING = 0;
QStringList XFMS_DATA::__SL_FINISHED;
QString XFMS_DATA::__ERROR_LOADING;

int XFMS_DATA::dat; //Current date

std::multimap<QString, NVUPOINT*> XFMS_DATA::lWP;
std::multimap<QString, NVUPOINT*> XFMS_DATA::lWP2;

//VOR/DME loaded from X-Plane earth earth_nav.dat, as it includes VOR angle deviation data.
std::vector<NVUPOINT*> XFMS_DATA::lXVORDME;

//Loaded from rsbn.dat
std::vector<NVUPOINT*> XFMS_DATA::lRSBN;

//Loaded from xnvu.dat
std::vector<NVUPOINT*> XFMS_DATA::lXNVU;

XFMS_DATA::XFMS_DATA()
{

}

XFMS_DATA::~XFMS_DATA()
{
    clear();
}

void XFMS_DATA::clear()
{
    lXVORDME.clear();
    lRSBN.clear();
    lXNVU.clear();
    lWP2.clear();

    std::multimap<QString, NVUPOINT*>::iterator it = lWP.begin();
    for(; it!=lWP.end(); it++)
    {
        delete it->second;
    }
    lWP.clear();
}

void XFMS_DATA::setDate(int _dat)
{
    dat = _dat;

    std::multimap<QString, NVUPOINT*>::iterator it = lWP.begin();

    for(; it!=lWP.end(); it++)
    {
        NVUPOINT* cP = (*it).second;
        double elev = 0;
        /*
        if(cP->wpOrigin == WAYPOINT::ORIGIN_AIRAC_AIRPORTS ||
           cP->wpOrigin == WAYPOINT::ORIGIN_AIRAC_NAVAIDS ||
           cP->wpOrigin == WAYPOINT::ORIGIN_EARTHNAV ||
           cP->wpOrigin == WAYPOINT::ORIGIN_XNVU)
        {
        */
            elev = (double(LMATH::feetToMeter(cP->elev))/1000.0);
        //}
        cP->MD = calc_magvar(cP->latlon.x, cP->latlon.y, dat, elev);
    }
}

//ICAO_name2 defines if user want to search in both ICAO name and full name of waypoint, or either:
//0: Search both
//1: Only ICAO
//2: Only full name
std::vector<NVUPOINT*> XFMS_DATA::search(const QString& _name, int _type, int _ICAO_name2, const QString& _country)
{
    std::vector<NVUPOINT*> rWP;
    std::pair<std::multimap<QString, NVUPOINT*>::iterator, std::multimap<QString, NVUPOINT*>::iterator> ret;

    if(_name.isEmpty()) return rWP;
    if(_ICAO_name2>0 && _ICAO_name2 != 1) goto _name2;  //I hate GOTO:s, but they are effective.

    ret = lWP.equal_range(_name);
    for(std::multimap<QString, NVUPOINT*>::iterator it=ret.first; it!=ret.second; it++)
	{
        NVUPOINT* wp = it->second;
        if(_type>0 && wp->type != _type) continue;
        if(_country!=NULL && !_country.isEmpty()) if(_country.compare(wp->country) !=0) continue;
        rWP.push_back(wp);
	}

    _name2:
    if(_ICAO_name2>0 && _ICAO_name2 != 2) return rWP;

    ret = lWP2.equal_range(_name);
    for(std::multimap<QString, NVUPOINT*>::iterator it=ret.first; it!=ret.second; it++)
    {
        NVUPOINT* wp = it->second;
        if(_type>0 && wp->type != _type) continue;
        if(_country!=NULL && !_country.isEmpty()) if(_country.compare(wp->country) !=0) continue;
        rWP.push_back(wp);
    }

    return rWP;
}

QString XFMS_DATA::getAirwayWaypointsBetween(QString& airway, NVUPOINT* wpA, NVUPOINT* wpB, std::vector<NVUPOINT*>& lA, NVUPOINT* &_wpAirway, bool allowOpposite)
{
    std::vector<NVUPOINT*> llA = XFMS_DATA::search(airway); //Get a list of all airways with the same identifier (hence the airways are directional, and should logically share the same waypoints)
    QString sError = "";
    bool kAIsFound = false;
    bool kBIsFound = false;
    _wpAirway = NULL;

    lA.clear();
    for(int i=0; i<llA.size(); i++)
    {
        NVUPOINT* wp = llA[i];
        if(wp->type != WAYPOINT::TYPE_AIRWAY) continue;

        AIRWAY* aw = (AIRWAY*) wp->data;
        if(!aw) continue;

        int kA = -1;
        int kB = -1;
        for(int k=0; k<aw->lATS.size(); k++)    //Search for the identifers in the airway (where the airway should start and end).
        {
            NVUPOINT* ap = (NVUPOINT*) aw->lATS[k];
            if(ap->name.compare(wpA->name)==0) kA = k;
            if(ap->name.compare(wpB->name)==0) kB = k;
        }

        if(kA>=0) kAIsFound = true;
        if(kB>=0) kBIsFound = true;

        if(kA<0 || kB<0) continue;  //If identifiers is not found, continue to next airway in list.
        else if(kB<kA)              //If wpA is after wpB, and allowing the opposite direction of a directional airway
        {
            if(lA.size()==0 && allowOpposite)
            {
                for(int k=kA; k>=kB; k--)
                {
                    lA.push_back((NVUPOINT*) aw->lATS[k]);
                }
                _wpAirway = wp;
                sError = "";
            }//if
            else sError = "Route: Note, airway [" + airway + "] is directional and opposite to waypoints [" + wpA->name + "] -> [" + wpB->name + "].";
            continue;
        }
        else if(kA<kB)  //If wpB is after wpA
        {
            lA.clear();
            for(int k=kA; k<=kB; k++)
            {
                lA.push_back((NVUPOINT*) aw->lATS[k]);
            }
            _wpAirway = wp;
            return "";
        }
    }//for

    if(llA.size()==0)
    {
        _wpAirway = NULL;
        return "Route: Airway [" + airway + "] is not found.";
    }

    //If nothing is added something is wrong. Set error returns here.
    if(sError.isEmpty() && lA.size()==0)
    {
        _wpAirway = NULL;
        if(kAIsFound && kBIsFound)
        {
            return "Route: Cannot connect waypoints [" + wpA->name + "] and [" + wpB->name + "].";
        }

        if(!kAIsFound && !kBIsFound) return "Route: Waypoint [" + wpA->name + "] and [" + wpB->name + "] is not found in airway [" + airway + "].";
        if(!kAIsFound) return "Route: Waypoint [" + wpA->name + "] is not found in airway [" + airway + "].";
        else if(!kBIsFound) return "Route: Waypoint [" + wpB->name + "] is not found in airway [" + airway + "].";

        return "Route: Major programming error, should not be here, contact confused developer.";
    }

    if(!sError.isEmpty()) _wpAirway = NULL;
    return sError;
}

//Allocates and returns a list of waypoints. Caller needs to delete the waypoints when not used anymore.
//TODO: Make a smart algorithm for deciding which single waypoints should be added, hence there are different
//waypoints with the same identifer, and it is not enough to check the neareast waypoint
//(see airway 'R487' and waypoint 'OK').
QString XFMS_DATA::getRoute(const QString& _qstr, std::vector<NVUPOINT*>& _route, NVUPOINT* wpRef)
{
    QStringList record = _qstr.split(' ', QString::SkipEmptyParts);
    std::vector<NVUPOINT*> sWaypoint;
    std::vector<NVUPOINT*> cRoute;
    std::vector<NVUPOINT*> route;
    std::vector<NVUPOINT*> pBuffer; //Buffer of custom created waypoints to delete if error.

    QString qstr;
    QString sError;
    NVUPOINT* cwp;
    double dCurrent, dMin;
    AIRWAY* awyA = NULL;
    AIRWAY* awyB = NULL;
    int pBetween = 0;



    route.clear();
    for(int i=0; i<record.size(); i++)
    {
        qstr = record[i];

        //Search for identifier in database, if not found, add if this is a custom waypoint.
        sWaypoint = XFMS_DATA::search(qstr, 0, 1);
        if(sWaypoint.size() == 0)
        {
            //TODO If there are more records, and this is the first one, set wpRef to next waypoint, and calculate later.
            if(cRoute.size() == 0) cwp = wpRef;
            else cwp = cRoute[cRoute.size() - 1];

            NVUPOINT* wp = NULL;
            sError = validate_custom_point(cwp, wp, qstr);
            if(wp)
            {
                pBuffer.push_back(wp);
                cRoute.push_back(wp);
                continue;
            }
            else
            {
                sError = "Route: " + sError;
                goto rError;
            }
         }//if

        //If more than 1 waypoint is found, choose the waypoint that is closest to the last on in current route.
        dMin = std::numeric_limits<double>::max();
        dCurrent = 0;
        for(int k=0; k<sWaypoint.size(); k++)
        {
            if(cRoute.size()>0) dCurrent = LMATH::calc_distance(sWaypoint[k]->latlon, cRoute[cRoute.size()-1]->latlon);
            else if(wpRef) dCurrent = LMATH::calc_distance(sWaypoint[k]->latlon, wpRef->latlon);
            if(dCurrent<dMin)
            {
                dMin = dCurrent;
                cwp = sWaypoint[k];
            }
        }

        cRoute.push_back(cwp);
    }
    sWaypoint.clear();

    for(int i=0; i<cRoute.size(); i++)
    {
        NVUPOINT* wp = cRoute[i];

        //If waypoint is an airway, check if previous and next waypoint in route exist in airway,
        if(wp->type == WAYPOINT::TYPE_AIRWAY)
        {
            AIRWAY* ats = (AIRWAY*) wp->data;
            if(!ats) continue;
            //It is not valid if an airway starts or ends the route
            if(i==0)
            {
                sError = "Route: First point in route should not be an airway [" + ats->name + "].";
                goto rError;
            }
            if(i==(int(cRoute.size())-1))
            {
                sError = "Route: Last point in route should not be an airway [" + ats->name + "].";
                goto rError;
            }//if


            std::vector<NVUPOINT*> lA;
            NVUPOINT* rPrev = cRoute[i-1];
            NVUPOINT* rNext = cRoute[i+1];
            NVUPOINT* wpAirway = NULL;
            sError = XFMS_DATA::getAirwayWaypointsBetween(ats->name, rPrev, rNext, lA, wpAirway, true);

            //Return error
            if(!sError.isEmpty())
            {
                goto rError;
            }

            //Set the correct airway in cRoute for comparsion algorithm in the for loop below
            if(wpAirway) cRoute[i] = wpAirway;

            //Set the correct waypoints before and after airway (for comparsion algorithm in the for loop below).
            if(lA.size()>0)
            {
                cRoute[i-1] = lA[0];
                cRoute[i+1] = lA[lA.size()-1];
            }//if

            route.pop_back();   //Remove the last waypoint, as a new and more correct one is added from the airway.
            for(int k=0; k<lA.size(); k++)
            {
                route.push_back(lA[k]);
            }//for
            i++;      //Step forward as we already added the next waypoint
            continue; //Do not push the airway itself to the list
        }//if

        /*
        else if(wp->type==WAYPOINT::ORIGIN_FLIGHTPLAN)
        {
            route.push_back(wp);
            continue;
        }

        wp->wpOrigin = WAYPOINT::ORIGIN_FLIGHTPLAN;
        */

        route.push_back(wp);
    }//for

    //Now the route is finished. First though we need to check if the start and end points
    //of connecting airways are the same, if not there is an error in route.
    //I.e. route R487 and W234 may share the same waypoint identifer between, but the waypoint
    //itself may be duplicate and placed on several places in world.
    for(unsigned int i=0; i<cRoute.size(); i++)
    {
        if(cRoute[i]->type == WAYPOINT::TYPE_AIRWAY)
        {
            if(awyA && pBetween == 1)
            {
                awyB = (AIRWAY*) cRoute[i]->data;
                NVUPOINT* wp = cRoute[i-1];

                //Check if waypoint is the same waypoint in both airways
                bool okA = false;
                bool okB = false;
                for(unsigned int j=0; j<awyA->lATS.size(); j++)
                {
                    if(wp == awyA->lATS[j])
                    {
                       okA = true;
                       break;
                    }
                }//for
                for(unsigned int j=0; j<awyB->lATS.size(); j++)
                {
                    if(wp == awyB->lATS[j])
                    {
                       okB = true;
                       break;
                    }
                }//for

                if(okA == false || okB == false)
                {
                    sError = "Route: Waypoint [" + wp->name + "] cannot join airway [" + awyA->name + "] and [" + awyB->name + "].";
                    goto rError;
                }

                awyA = NULL;
                awyB = NULL;
                pBetween = 0;
                i--;
                continue;
            }//if
            else
            {
                awyA = (AIRWAY*) cRoute[i]->data;
                pBetween = 0;
            }//else
        }//if
        else
        {
            pBetween++;
        }//eöse

    }//for

    //Allocate and copy the route to flightplan
    for(unsigned int i=0; i<route.size(); i++)
    {
        _route.push_back(new NVUPOINT(*route[i]));
    }

    return "";

    //Deallocate custom waypoints and return error.
    rError:
        for(unsigned int i=0; i<pBuffer.size(); i++) delete pBuffer[i];
        cRoute.clear();
        route.clear();
        return sError;
}

//Get closest waypoints of same type and name. Returns a list of found waypoints and their distances.
std::vector< std::pair<NVUPOINT*, double> > XFMS_DATA::getClosestWaypointType(const CPoint& _latlon, const QString& _name, int _type)//, double &distance)
{
    std::vector< std::pair<NVUPOINT*, double> > lWP;
    double dMin = std::numeric_limits<double>::max();
    std::vector<NVUPOINT*> lS = search(_name, _type);

    for(int i=0; i<lS.size(); i++)
    {
        double d = LMATH::calc_distance(_latlon, lS[i]->latlon);
        if(d<=dMin)
        {
            lWP.push_back(std::make_pair(lS[i], d));
        }
    }

    //Sorting the distance of found waypoints
    sort(lWP.begin(), lWP.end(),
    [](const std::pair<NVUPOINT*, double>& lhs, const std::pair<NVUPOINT*, double>& rhs) -> bool
    {
        if(lhs.second == 0) return true;
        return lhs.second < rhs.second;
    });

    return lWP;
}

//Closest to waypoint, N closest RSBN to return, D max distance of RSBN, includeVOR to true to include VORDME
//If N<1, return all the RSBN:s
std::vector< std::pair<NVUPOINT*, double> > XFMS_DATA::getClosestRSBN(const NVUPOINT* wp, int n, double d, bool includeVOR)
{
    if(n<0) n = lRSBN.size() + ((includeVOR==true) ? lXVORDME.size() : 0);

    if(d<0) d = std::numeric_limits<double>::max();

    std::vector< std::pair<NVUPOINT*, double> > rCP;
    for(int i=0; i<lRSBN.size(); i++)
    {
        double cD = LMATH::calc_distance(wp->latlon.x, wp->latlon.y, lRSBN[i]->latlon.x, lRSBN[i]->latlon.y);
        if(cD>d) continue;
        rCP.push_back(std::make_pair(lRSBN[i], cD));
    }

    if(includeVOR) for(int i=0; i<lXVORDME.size(); i++)
    {
        double cD = LMATH::calc_distance(wp->latlon.x, wp->latlon.y, lXVORDME[i]->latlon.x, lXVORDME[i]->latlon.y);
        if(cD>d) continue;
        rCP.push_back(std::make_pair(lXVORDME[i], cD));
    }


    sort(rCP.begin(), rCP.end(),
    [](const std::pair<NVUPOINT*, double>& lhs, const std::pair<NVUPOINT*, double>& rhs) -> bool
    {
        if(lhs.second == 0) return true;
        return lhs.second < rhs.second;
    });


    while(rCP.size()>n) rCP.pop_back();

    return rCP;
}


int XFMS_DATA::_loadXP10(const QString& file, int type)
{

    QFile infile(file);
    if(!infile.open(QIODevice::ReadOnly | QIODevice::Text)) return 0;

    if(type == 6)
    {
        validate_airways_XP10(infile);
        infile.close();
        return 1;
    }

    while(!infile.atEnd())
	{
        QString line = infile.readLine();
        QStringList list;
        if(type == 3 || type == 5) list = line.split('|');//, QString::SkipEmptyParts);                 //RSBN data (rsbn.dat) and xnvu_wps.txt
        else if(type == 4) list = line.split(' ', QString::SkipEmptyParts);     //X-Plane navdata (earth_nav.dat) and all XP11 files
        else list = line.split(',',  QString::SkipEmptyParts);                                          //XP10 other files

        switch(type) //else
        {
            case 0: validate_airport_XP10(list); break;
            case 1: validate_navaid_XP10(list); break;
            case 2: validate_waypoint_XP10(list); break;
            case 3: validate_RSBN(list); break;
            case 4: validate_earthnav_XP10(list); break;
            case 5: validate_xnvu(list); break;
        }//switch, else
	}//while

	infile.close();

    return 1;
}//void load

int XFMS_DATA::_loadAirportsXP11(QString& sError)
{
    //1. Default apt.dat: "Resources\default scenery\default apt dat\Earth nav data\apt.dat"
    //2. Gateway apt.dat (overrides default apt.dat): "Custom Scenery\Global Airports\Earth nav data\apt.dat"
    //3. Custom apt.dat (overrides both above): "Custom Scenery/XXXXX/Earth nav data\apt.dat"
    //We should load in above steps reversed, i.e. 3, 2 then 1 and ignoring previously added airports, using ICAO as identifier.
    sError = "";

    //1. Load custom airports first

    //TODO: Define DialogSettings::customAirportsDir
    //QDir dir(DialogSettings::customAirportsDir);
    QDir dir(DialogSettings::xDir + "/Custom Scenery");

    if(DialogSettings::XP11_includeCustomAirports)
    {
        QFileInfoList files = dir.entryInfoList();
        foreach(const QFileInfo &fi, files)
        {
            QString path = fi.absoluteFilePath();
            if(fi.baseName().compare("Global Airports") == 0) continue;
            if(fi.isDir())
            {
                path = path + "/Earth nav data/apt.dat";

                QFile inFile(path);
                if(!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) continue;   //apt.dat is not readable or found in custom airport folder
                validate_airports_XP11(inFile, WAYPOINT::ORIGIN_X11_CUSTOM_AIRPORTS);
                inFile.close();
            }//if
        }//foreach
    }//if


    //2. Load gateway apt.dat
    QString path = DialogSettings::xDir + "/Custom Scenery/Global Airports/Earth nav data/apt.dat";
    QFile inFile(path);
    if(inFile.open(QIODevice::ReadOnly | QIODevice::Text))   //If apt.dat is readable and found
    {
        validate_airports_XP11(inFile, WAYPOINT::ORIGIN_X11_GATEWAY_AIRPORTS);
    }
    else sError = "[GATEWAY apt.dat]";
    inFile.close();



    //3. Load default apt.dat
    path = DialogSettings::xDir + "/Resources/default scenery/default apt dat/Earth nav data/apt.dat";
    inFile.setFileName(path);
    if(inFile.open(QIODevice::ReadOnly | QIODevice::Text))   //If apt.dat is readable and found
    {
        validate_airports_XP11(inFile, WAYPOINT::ORIGIN_X11_DEFAULT_AIRPORTS);
    }
    else
    {
        if(sError.length()>0) sError = "[DEFAULT & GATEWAY apt.dat]";
        else sError = "[DEFAULT apt.dat]";
    }
    inFile.close();

    return 1;
}

int XFMS_DATA::_loadWaypointsXP11(QString& sError)
{
    sError = "";


    //Read custom earth_fix.dat
    QString path = DialogSettings::xDir + "/Custom Data/earth_fix.dat";
    QFile inFile(path);

    if(inFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while(!inFile.atEnd())
        {
            QString line = inFile.readLine();
            QStringList list;
            list = line.split(' ', QString::SkipEmptyParts);
            validate_waypoint_XP11(list, WAYPOINT::ORIGIN_X11_CUSTOM_FIXES);
        }//while
        inFile.close();
        return 1;
    }
    inFile.close();

    //If custom earth_fix.dat is not found, read default earth_fix.dat

    path = DialogSettings::xDir + "/Resources/default data/earth_fix.dat";
    inFile.setFileName(path);
    if(inFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while(!inFile.atEnd())
        {
            QString line = inFile.readLine();
            QStringList list;
            list = line.split(' ', QString::SkipEmptyParts);
            validate_waypoint_XP11(list, WAYPOINT::ORIGIN_X11_DEFAULT_FIXES);
        }//while

        inFile.close();
        return 1;
    }

    inFile.close();
    sError = "[earth_fix.dat]";
    return 0;
}

int XFMS_DATA::_loadNavDataXP11(QString& sError)
{
    sError = "";
    QString path = DialogSettings::xDir + "/Custom Data/earth_nav.dat";
    QFile inFile(path);

    if(inFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while(!inFile.atEnd())
        {
            QString line = inFile.readLine();
            QStringList list;
            list = line.split(' ', QString::SkipEmptyParts);
            validate_earthnav_XP11(list, WAYPOINT::ORIGIN_X11_CUSTOM_EARTHNAV);
        }//while

        inFile.close();
        return 1;
    }
    inFile.close();

    path = DialogSettings::xDir + "/Resources/default data/earth_nav.dat";
    inFile.setFileName(path);
    if(inFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while(!inFile.atEnd())
        {
            QString line = inFile.readLine();
            QStringList list;
            list = line.split(' ', QString::SkipEmptyParts);
            validate_earthnav_XP11(list, WAYPOINT::ORIGIN_X11_DEFAULT_EARTHNAV);
        }//while

        inFile.close();
        return 1;
    }

    inFile.close();
    sError = "[earth_nav.dat]";
    return 0;
}

int XFMS_DATA::_loadAirwaysXP11(QString& sError)
{
    sError = "";
    QString path = DialogSettings::xDir + "/Custom Data/earth_awy.dat";
    QFile inFile(path);

    if(inFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        validate_airways_XP11(inFile, WAYPOINT::ORIGIN_X11_CUSTOM_ATS);
        inFile.close();
        return 1;
    }
    inFile.close();

    path = DialogSettings::xDir + "/Resources/default data/earth_awy.dat";
    inFile.setFileName(path);
    if(inFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        validate_airways_XP11(inFile, WAYPOINT::ORIGIN_X11_DEFAULT_ATS);
        inFile.close();
        return 1;
    }

    inFile.close();
    sError = "[earth_awy.dat]";
    return 0;
}

void XFMS_DATA::load(int _dat)
{
    QString sError;
    QString rError;
    dat = _dat;

    __SL_FINISHED.clear();
    __CURRENT_LOADING = 0;
    for(int i=0; i<8; i++) __DATA_LOADED[i] = 0;
    __ERROR_LOADING = "";

    if(DialogSettings::XP11)
    {
        if(!_loadAirportsXP11(sError))
        {
            //1. Default apt.dat: "Resources\default scenery\default apt dat\Earth nav data\apt.dat"
            //2. Gateway apt.dat (overrides default apt.dat): "Custom Scenery\Global Airports\Earth nav data\apt.dat"
            //3. Custom apt.dat (overrides both above): "Custom Scenery/XXXXX/Earth nav data\apt.dat"
            //We should load in above steps reversed, i.e. 3, 2 then 1 and ignoring previously added airports, using ICAO as identifier.
        }

        __CURRENT_LOADING++;
        if(!_loadWaypointsXP11(rError))
        {
            //1. "Resources/default data/earth_fix.dat"
            //2. "Custom Data/earth_fix.dat"
            //If number 2 exist, we ignore number 1.
            sError = sError + " " + rError;
        }

        __CURRENT_LOADING++;
        if(!_loadNavDataXP11(rError))
        {
            //1. "Resources/default data/earth_nav.dat"
            //2. "Custom Data/earth_nav.dat"
            //3. "Custom Scenery/Global Airports/Earth nav data/earth_nav.dat"
            //We do load the ILS beacons from 3 first. If number 2 exist, we ignore number 1. Then we load 1 or 2 and ignore ILS beacons which we already have added from 3.
            //EDIT: Change, we skip loading ILS beacons (step 3) for now.
            sError = sError + " " + rError;
        }

        __CURRENT_LOADING++;
        if(!_loadAirwaysXP11(rError))
        {
            //1. "Resources/default data/earth_awy.dat"
            //2. "Custom Data/earth_awy.dat"
            //If number 2 exist, we ignore number 1.
            sError = sError + " " + rError;
        }
    }
    else
    {
        if(!_loadXP10(DialogSettings::fileAirports, 0))
        {
            sError = " [Airports.txt]";
        }

        __CURRENT_LOADING++;
        if(!_loadXP10(DialogSettings::fileNavaids, 1))
        {
            sError = sError + " [Navaids.txt]";
        }

        __CURRENT_LOADING++;
        if(!_loadXP10(DialogSettings::fileWaypoints, 2))
        {
            sError = sError + " [Waypoints.txt]";
        }

        __CURRENT_LOADING++;
        if(!_loadXP10(DialogSettings::fileNavdata, 4))
        {
            sError = sError + " [earth_nav.txt]";
        }

        __CURRENT_LOADING++;
        if(!_loadXP10(DialogSettings::fileAirways, 6))
        {
            sError = sError + " [ats.txt]";
        }
    }//else

    __CURRENT_LOADING++;
    if(!_loadXP10(DialogSettings::fileRSBN, 3))
    {
        sError = sError + " [rsbn.dat]";
    }

    __CURRENT_LOADING++;
    if(!_loadXP10(XNVU_WPS_FILENAME, 5))
    {
        sError = sError + " [xnvu_wps.txt]";

    }

    __CURRENT_LOADING++;

   __ERROR_LOADING = sError;
}

void XFMS_DATA::addXNVUData(std::vector<NVUPOINT*> lP)
{
    for(int i=0; i<lP.size(); i++)
    {
        addXNVUWaypoint(lP[i]);
    }
}

void XFMS_DATA::addXNVUWaypoint(NVUPOINT* lP)
{
    lXNVU.push_back(lP);
    lWP.insert(std::make_pair(lP->name, lP));
    lWP2.insert(std::make_pair(lP->name2, lP));
}

void XFMS_DATA::removeXNVUWaypoint(NVUPOINT* lP)
{
    std::vector<NVUPOINT*>::iterator iL;
    std::multimap<QString, NVUPOINT*>::iterator iM;

    for(iL = lXNVU.begin(); iL!=lXNVU.end(); iL++)
    {
        if(*iL == lP)
        {
            lXNVU.erase(iL);
            break;
        }
    }

    //Remove points from lWP and lWP2
    for(iM = lWP.begin(); iM!=lWP.end(); iM++)
    {
        if((*iM).second == lP)
        {
            lWP.erase(iM);
            break;
        }//if
    }

    for(iM = lWP2.begin(); iM!=lWP2.end(); iM++)
    {
        if((*iM).second == lP)
        {
            lWP2.erase(iM);
            break;
        }//if
    }
}

void XFMS_DATA::validate_airports_XP11(QFile& infile, int wpOrigin)
{
    /* The new apt.dat format gives elevation and region data with these tags:
     * 1302 datum_lat
     * 1302 datum_lon
     * 1302 icao_code
     * 1302 faa_code
     * 1302 region_code
     * If the datum do not exist, take middle on the first runway (100/101), or the first helipad of no runway exist (102).
     */
    QString line;
    QStringList record;

    /* currentState values:
     * 0: Searching for airport header
     * 1: Airport header
     * 2: Searching for 1302 datum_xx or 100-102
     * 1302: 1302 datum_xx header
     * 100: 1st 100 header
     * 102: 1st 101 header
     * 103: 1st 102 header
     */
    int currentState = 0;
    QString qstr;
    int cmpData;
    bool lat_set = false;
    bool lon_set = false;
    CPoint latA, latB;
    int surfaceType = 15; //Transparent as default, hence unknown.
    QString rwyIdentifierA = "";
    QString rwyIdentifierB = "";
    double rwyLength = -1;
    double rwyWidth = -1;
    int lightA = 0;
    int lightB = 0;
    QString city = "";
    std::vector<RUNWAY*> lRunways;
    std::vector<std::pair<int, int> > lFreq;

    NVUPOINT wp;
    wp.type = WAYPOINT::TYPE_AIRPORT;

    while(!infile.atEnd())
    {
        line = infile.readLine();
        record = line.split(' ', QString::SkipEmptyParts);
        for(int i=0; i<record.size(); i++)
        {
            qstr = record[i].simplified();

            switch(i)
            {
                case 0: //Row type
                    cmpData = qstr.toInt();
                    if(cmpData == 1 || cmpData == 16 || cmpData == 17)
                    {
                        //If first item in row is 1, 16 or 17, it tells us this is the start of an airport data.
                        //We set the currentState to 1 to show we begin with a new initialization of an airport.

                        //Note that similar code is also placed in the end of this function, to grab the last found airport.
                        wp.wpOrigin = wpOrigin;
                        wp.type = WAYPOINT::TYPE_AIRPORT;
                        wp.data = NULL;

                        AIRPORT_DATA* arwy = new AIRPORT_DATA();
                        //for(unsigned int j=0; j<lRunways.size(); j++) arwy->lRunways.push_back(lRunways[j]);
                        for(unsigned int j=0; j<lFreq.size(); j++) arwy->lFreq.push_back(lFreq[j]);
                        arwy->city = city;

                        if(lWP.find(wp.name) != lWP.end() || lRunways.size()<1)  //If airport already exist, delete current allocated data and skip this airport
                        {
                            for(unsigned int j = 0; j<lRunways.size(); j++) delete lRunways[j];
                            delete arwy;
                        }
                        else
                        {
                            RUNWAY* rwy;
                            double lMax = -1;
                            for(unsigned int j = 0; j<lRunways.size(); j++) if(lRunways[j]->getLength()>lMax){ rwy = lRunways[j]; lMax = lRunways[j]->getLength(); }
                            wp.length = lMax;
                            if(!lat_set || !lon_set) wp.latlon = rwy->getMiddlePoint();

                            NVUPOINT* nwp = new NVUPOINT(wp);
                            nwp->MD = calc_magvar(nwp->latlon.x, nwp->latlon.y, dat, (double(LMATH::feetToMeter(nwp->elev))/1000.0));
                            nwp->data = (void*) arwy;
                            validate_runways(nwp, lRunways);
                            for(unsigned int j = 0; j<lRunways.size(); j++) delete lRunways[j];
                            lWP.insert(std::make_pair(nwp->name, nwp));
                            lWP2.insert(std::make_pair(nwp->name2, nwp));
                            __DATA_LOADED[__CURRENT_LOADING]++;
                        }//if

                        //Reset the flags and continue with this new airport
                        lFreq.clear();
                        lRunways.clear();
                        wp = NVUPOINT();
                        lat_set = 0;
                        lon_set = 0;
                        rwyLength = -1.0;
                        rwyWidth = -1.0;
                        surfaceType = 15;
                        rwyIdentifierA = "";
                        rwyIdentifierB = "";
                        city = "";
                        lightA = 0;
                        lightB = 0;
                        currentState = 1;
                        continue;
                    }
                    else if(cmpData == 1302 || (cmpData>99 && cmpData<103) || (cmpData>49 && cmpData<57)) currentState = cmpData;
                    else i = record.size(); //Nothing to see here, begone!
                break;

                case 1: //(1) Elevation, (1302) datum_xxx
                    if(currentState == 1) //Elevation
                    {
                        wp.elev = qstr.toInt();
                    }
                    else if(currentState>49 && currentState<57) //ATC FREQ
                    {
                        lFreq.push_back(std::make_pair(currentState, qstr.toInt()));
                    }
                    else if(currentState == 100 || currentState == 101)
                    {
                        rwyWidth = qstr.toDouble();
                    }
                    else if(currentState == 102)
                    {
                        rwyIdentifierA = qstr;
                    }
                    else if(currentState == 1302)
                    {
                        if(i<(int(record.size())-1))
                        {
                            i++;
                            if(qstr.compare("datum_lat") == 0)
                            {
                                if(i>=record[i].size()) break;
                                qstr = record[i].simplified();
                                wp.latlon.x = qstr.toDouble();
                                lat_set = true;
                            }
                            else if(qstr.compare("datum_lon") == 0)
                            {
                                if(i>=record[i].size()) break;
                                qstr = record[i].simplified();
                                wp.latlon.y = qstr.toDouble();
                                lon_set = true;
                            }//if
                            else if(qstr.compare("region_code") == 0)
                            {
                                if(i>=record[i].size()) break;
                                qstr = record[i].simplified();
                                wp.country = qstr;
                                break;
                            }
                            else if(qstr.compare("transition_alt") == 0)
                            {
                                if(i>=record[i].size()) break;
                                qstr = record[i].simplified();
                                wp.trans_alt = qstr.toInt();
                                break;
                            }
                            else if(qstr.compare("transition_alt") == 0)
                            {
                                if(i>=record[i].size()) break;
                                qstr = record[i].simplified();
                                wp.trans_level = qstr.toInt();
                                break;
                            }
                            else if(qstr.compare("city") == 0)
                            {
                                if(i>=record[i].size()) break;
                                city = record[i].simplified();
                            }
                        }

                        i = record.size(); //We have managed to add all data for this row.
                    }
                break;

                case 2: // (1) Deprecated, (100) Runway surface, (102) Helipad longitude
                    //if(currentState == 102 &&!latAB_set) wp.latlon.x = qstr.toDouble();
                    if(currentState == 100) surfaceType = qstr.toInt();
                    else if(currentState == 102) latA.x = qstr.toDouble();
                break;

                case 3: // (1) Deprecated, (101) Water runway identifier, (102) Helipad latitude
                    if(currentState == 101)
                    {
                        rwyIdentifierA = qstr;
                    }
                    else if(currentState == 102)
                    {
                        latA.y = qstr.toDouble();
                    }//if
                break;

                case 4: // (1) ICAO code, (101) Water runway start longitude
                    if(currentState == 1)
                    {
                        wp.name = qstr;
                    }//if
                    else if(currentState == 101) latA.x = qstr.toDouble();
                break;
                case 5: // (1) Airport name, (101) Water runway start latitude, (102) length of helipad
                    if(currentState == 1)
                    {
                        while(i<record.size())
                        {
                            if(wp.name2.isEmpty()) wp.name2 = qstr;
                            else wp.name2 = wp.name2 + " " + qstr;
                            i++;
                            if(i<record.size()) qstr = record[i].simplified();
                        }
                    }//if
                    else if(currentState == 101) latA.y = qstr.toDouble();
                    else if(currentState == 102) rwyLength = qstr.toDouble();
                break;
                case 6: //(101) Water runway end name, (102) width of helipad
                    if(currentState == 101) rwyIdentifierB = qstr;
                    else if(currentState == 102) rwyWidth = qstr.toDouble();
                break;
                case 7: //(101) Water runway end longitude
                    if(currentState == 101) latB.x = qstr.toDouble();
                    else if(currentState == 102)
                    {
                        surfaceType = qstr.toInt();
                        RUNWAY* rwy = new RUNWAY(rwyIdentifierA, latA, rwyLength, rwyWidth, surfaceType, WAYPOINT::TYPE_HELIPAD);
                        lRunways.push_back(rwy);
                        currentState = 0;
                    }
                break;
                case 8: //(100), Runway identifer (101) Water runway end latitude
                   if(currentState == 100)
                   {
                       rwyIdentifierA = qstr;
                   }
                   if(currentState == 101)
                   {
                       latB.y = qstr.toDouble();
                       RUNWAY* rwy = new RUNWAY(rwyIdentifierA, latA, 0, rwyIdentifierB, latB, 0, rwyWidth, SURFACE_WATER, WAYPOINT::TYPE_RUNWAY);
                       lRunways.push_back(rwy);
                       currentState = 0;
                   }
                break;
                case 9: //(100) Runway start longitude
                    if(currentState == 100) latA.x = qstr.toDouble();
                break;
                case 10: //(100) Runway start latitude
                    if(currentState == 100) latA.y = qstr.toDouble();
                break;
                case 14: //(100) Runway approach lightning
                    if(currentState == 100) lightA = qstr.toInt();
                break;
                case 17: //(100) Runway end identifier
                    if(currentState == 100) rwyIdentifierB = qstr;
                break;
                case 18: //(100) Runway end longitude
                    if(currentState == 100) latB.x = qstr.toDouble();
                break;
                case 19: //(100) Runway end latitude
                    if(currentState == 100)
                    {
                        latB.y = qstr.toDouble();
                    }//if
                break;
                case 23: //(100) Runway end approach lightning
                    if(currentState == 100)
                    {
                        lightB = qstr.toInt();
                        RUNWAY* rwy = new RUNWAY(rwyIdentifierA, latA, lightA, rwyIdentifierB, latB, lightB, rwyWidth, surfaceType, WAYPOINT::TYPE_RUNWAY);
                        lRunways.push_back(rwy);
                        currentState = 0;
                    }
                break;
            }
        }
    }//while

    //Add the last airport, if valid
    wp.wpOrigin = wpOrigin;
    wp.type = WAYPOINT::TYPE_AIRPORT;
    wp.data = NULL;

    AIRPORT_DATA* arwy = new AIRPORT_DATA();
    //for(unsigned int j=0; j<lRunways.size(); j++) arwy->lRunways.push_back(lRunways[j]);
    for(unsigned int j=0; j<lFreq.size(); j++) arwy->lFreq.push_back(lFreq[j]);
    arwy->city = city;

    if(lWP.find(wp.name) != lWP.end() || lRunways.size()<1)
    {
        for(unsigned int j = 0; j<lRunways.size(); j++) delete lRunways[j];
        delete arwy;
    }//if
    else
    {
        RUNWAY* rwy;
        double lMax = -1;
        for(unsigned int j = 0; j<lRunways.size(); j++) if(lRunways[j]->getLength()>lMax){ rwy = lRunways[j]; lMax = lRunways[j]->getLength(); }
        wp.length = lMax;
        if(!lat_set || !lon_set) wp.latlon = rwy->getMiddlePoint();

        NVUPOINT* nwp = new NVUPOINT(wp);
        nwp->MD = calc_magvar(nwp->latlon.x, nwp->latlon.y, dat, (double(LMATH::feetToMeter(nwp->elev))/1000.0));
        nwp->data = (void*) arwy;
        validate_runways(nwp, lRunways);
        for(unsigned int j = 0; j<lRunways.size(); j++) delete lRunways[j];
        lWP.insert(std::make_pair(nwp->name, nwp));
        lWP2.insert(std::make_pair(nwp->name2, nwp));
        __DATA_LOADED[__CURRENT_LOADING]++;
    }//if
}

void XFMS_DATA::validate_runways(const NVUPOINT* ap, const std::vector<RUNWAY*>& lRunways)
{
    AIRPORT_DATA* apd = (AIRPORT_DATA*) ap->data;
    if(!apd) return;

    NVUPOINT* rwy;
    for(unsigned int i=0; i<lRunways.size(); i++)
    {
        rwy = new NVUPOINT(*ap);
        if(rwy->data) delete rwy->data;
        //rwy->data = (void*) ap;
        rwy->data = NULL;
        rwy->latlon = lRunways[i]->start;
        rwy->length = lRunways[i]->getLength();
        rwy->width = lRunways[i]->getWidth();
        rwy->surface = lRunways[i]->surface;
        rwy->name = lRunways[i]->rwyA;
        rwy->name2 = ap->name;// + " - " + ap->name2;
        rwy->type = lRunways[i]->type;
        apd->lRunways.push_back(rwy);
        //lWP.insert(std::make_pair(rwy->name, rwy));
        //lWP2.insert(std::make_pair(rwy->name2, rwy));

        if(rwy->type == WAYPOINT::TYPE_HELIPAD) continue;

        rwy = new NVUPOINT(*ap);
        if(rwy->data) delete rwy->data;
        rwy->data = NULL;
        rwy->latlon = lRunways[i]->end;
        rwy->length = lRunways[i]->getLength();
        rwy->width = lRunways[i]->getWidth();
        rwy->surface = lRunways[i]->surface;
        rwy->name = lRunways[i]->rwyB;
        rwy->name2 = ap->name;// + " " + ap->name2;
        rwy->type = lRunways[i]->type;
        apd->lRunways.push_back(rwy);
        //lWP.insert(std::make_pair(rwy->name, rwy));
        //lWP2.insert(std::make_pair(rwy->name2, rwy));
    }
}

int XFMS_DATA::validate_header(QFile& infile)
{
    while(!infile.atEnd())
    {
        QStringList record;
        QString qstr;
        QString line;

        line = infile.readLine();
        record = line.split(' ',  QString::SkipEmptyParts);
        if(record.size()<6) continue;

        try
        {
            for(int i=0; i<record.size(); i++)
            {
                qstr = record.at(i).simplified();
                switch(i)
                {
                    case 0: //I.e. 1101
                        qstr.toInt();
                    case 1: //"Version"
                        if(qstr.compare("version", Qt::CaseInsensitive) != 0) continue;
                    case 2: //'-'
                        if(qstr.compare("-", Qt::CaseInsensitive) != 0) continue;
                    break;
                    case 3: //"data"
                        if(qstr.compare("data", Qt::CaseInsensitive) != 0) continue;
                    break;
                    case 4: //"cycle"
                        if(qstr.compare("cycle", Qt::CaseInsensitive) != 0) continue;
                    break;
                    case 5:
                        while(qstr.at(qstr.length()-1).isDigit() == false) qstr.chop(1);   //Remove comma, or letters if exist
                        int airacCycle = qstr.toInt();
                        return airacCycle;
                    break;
                }
            }
        }//try
        catch(...)
        {
            continue;
        }
    }//while

    return -2;
}

void XFMS_DATA::validate_cycle_info(QFile& infile, NAV_SOURCE_DATA &_navSource)
{
    _navSource.airacCycle = -1;
    _navSource.validDate = "";
    int currentState = -1;       //-1 Unknown, 0 AIRAC cycle, 1 version, 2 valid date
    while(!infile.atEnd())
    {
        QStringList record;
        QString qstr;
        QString line;

        line = infile.readLine();
        record = line.split(' ',  QString::SkipEmptyParts);
        if(record.size()<6) continue;

        try
        {
            for(int i=0; i<record.size(); i++)
            {
                qstr = record.at(i).simplified();
                switch(i)
                {
                    case 0:
                        if(qstr.compare("AIRAC", Qt::CaseInsensitive) == 0) currentState = 0;
                        else if(qstr.compare("Version", Qt::CaseInsensitive) == 0) currentState = 1;
                        else if(qstr.compare("Valid", Qt::CaseInsensitive) == 0) currentState = 2;
                        else continue;
                    break;
                    case 3:
                        switch(currentState)
                        {
                            case 0:     //AIRAC cycle
                                _navSource.airacCycle = qstr.toInt();
                                currentState = -1;
                            break;
                            case 1:     //Version
                                currentState = -1;
                            break;
                            case 2:     //Valid date
                                _navSource.validDate = "";
                                while(i<record.size())
                                {
                                    qstr = record.at(i).simplified();
                                    _navSource.validDate = _navSource.validDate + qstr + " ";
                                    i++;
                                }
                                _navSource.validDate.chop(1);
                                currentState = -1;
                            break;
                        }//switch
                    break;
                }//switch
            }//for
        }//try
        catch(...)
        {
            continue;
        }
    }//while
}

void XFMS_DATA::validate_waypoint_XP11(const QStringList& record, int _origin)
{

    QString test;
    if(record.size()<5) return;

    NVUPOINT* wp = new NVUPOINT();
    wp->type = WAYPOINT::TYPE_FIX;

    for(int i=0; i<record.size(); i++)
    {
        QString qstr = record.at(i).simplified();
        switch(i)
        {
            case 0: //Latitude
                wp->latlon.x = qstr.toDouble();
            break;
            case 1: //Longitude
                wp->latlon.y = qstr.toDouble();
            break;
            case 2: //ICAO code
                wp->name = qstr;
            break;
            case 3: //Airport terminal or ENRT (enroute)
                //wp->name2 = qstr;
            break;
            case 4: //ICAO region code or terminal area airport
                wp->country = qstr;
            break;
        }
    }

    wp->wpOrigin = _origin;
    wp->MD = calc_magvar(wp->latlon.x, wp->latlon.y, dat);

    //double d;
    std::vector< std::pair<NVUPOINT*, double> > wpSimilar = getClosestWaypointType(wp->latlon, wp->name, WAYPOINT::TYPE_FIX);
    bool isSimilarFound = false;
    for(unsigned int i=0; i<wpSimilar.size(); i++)
    {
        //if(wpSimilar && d<=DEFAULT_WAYPOINT_MARGIN)
        if(wpSimilar[i].second<=DEFAULT_WAYPOINT_MARGIN && wpSimilar[i].first->wpOrigin!=WAYPOINT::ORIGIN_XNVU)
        {
            isSimilarFound = true;
            break;
            /*
            if(wpSimilar->wpOrigin!=WAYPOINT::ORIGIN_XNVU)
            {
                delete wp;
                return;
            }
            */
        }
    }//for

    if(isSimilarFound)
    {
        delete wp;
        return;
    }


    lWP.insert(std::make_pair(wp->name, wp));
    lWP2.insert(std::make_pair(wp->name2, wp));
    __DATA_LOADED[__CURRENT_LOADING]++;
    //lFixes.push_back(wp);
}

void XFMS_DATA::validate_airways_XP11(QFile& infile, int _origin)
{
    //Line: Fix | Region | Type | Fix | Region | Type | Directional | Airway (Low or High) | Base | Top | Airway-Airway-Airway....

    std::vector<AIRWAY_SEGMENT_X11*> lASegs;

    while(!infile.atEnd())
    {
        bool bError = false;
        NVUPOINT wp;
        NVUPOINT wp2;
        AIRWAY_SEGMENT_X11 aSeg;
        int type;
        QStringList sharedAirways;
        QStringList record;
        QString qstr;
        QString line;

        line = infile.readLine();
        record = line.split(' ',  QString::SkipEmptyParts);
        try
        {
            for(int i=0; i<record.size(); i++)
            {
                qstr = record[i].simplified();
                switch(i)
                {
                case 0: //Fix ICAO
                    wp.name = qstr;
                    break;
                case 1: //Region i.e. ES
                    wp.country = qstr;
                    break;
                case 2: //Type of fix 11=fix, 2=enroute NDB, 3 = VOR, TACAN or DME
                    type = qstr.toInt();
                    if(type == 11) wp.type = WAYPOINT::TYPE_FIX;
                    else if(type == 2) wp.type = WAYPOINT::TYPE_NDB;
                    else if(type == 3) wp.type = WAYPOINT::TYPE_VHFNAV;
                    else bError = true;
                    break;
                case 3: //Fix ICAO
                    wp2.name = qstr;
                    break;
                case 4: //Region i.e. ES
                    wp2.country = qstr;
                    break;
                case 5: //Type of fix 11=fix, 2=enroute NDB, 3 = VOR, TACAN or DME
                    type = qstr.toInt();
                    if(type == 11) wp2.type = WAYPOINT::TYPE_FIX;
                    else if(type == 2) wp2.type = WAYPOINT::TYPE_NDB;
                    else if(type == 3) wp2.type = WAYPOINT::TYPE_VHFNAV;
                    else bError = true;
                    break;
                case 6: //Directional (N, F, B)
                    if(qstr.compare("N", Qt::CaseInsensitive) == 0) aSeg.direction = 0;
                    else if(qstr.compare("F", Qt::CaseInsensitive) == 0) aSeg.direction = 1;
                    else if(qstr.compare("B", Qt::CaseInsensitive) == 0) aSeg.direction = 2;
                    else bError = true;
                    break;
                case 7: //Airway low (1) or high (2)
                    aSeg.hilo = qstr.toInt();
                    break;
                case 8: //Base of airway FL
                    aSeg.base = qstr.toInt();
                    break;
                case 9: //Top of airway FL
                    aSeg.top = qstr.toInt();
                    break;
                case 10:
                    //aSeg.sharedAirways = qstr.split('-', QString::SkipEmptyParts);
                    sharedAirways = qstr.split('-', QString::SkipEmptyParts);
                    break;
                }//switch
            }//for
        }//try
        catch(...)
        {
            continue;
        }//catch

        if(bError) continue;


        //We set aSeg.start and aSeg.end, if the waypoint is not found in the global waypoint list (lWP), we ignore this segment.--------------------------------------------------------------
        std::vector<NVUPOINT*> sPoints;
        aSeg.start = NULL;
        sPoints = search(wp.name, 0, 1, wp.country);
        for(int i=0; i<sPoints.size(); i++)
        {
            if(sPoints[i]->wpOrigin!=WAYPOINT::ORIGIN_XNVU)
            {
                if(wp.type == WAYPOINT::TYPE_VHFNAV)
                {
                    if(sPoints[i]->type == WAYPOINT::TYPE_VORDME || sPoints[i]->type == WAYPOINT::TYPE_VOR ||
                       sPoints[i]->type == WAYPOINT::TYPE_TACAN || sPoints[i]->type == WAYPOINT::TYPE_VORTAC ||
                       sPoints[i]->type == WAYPOINT::TYPE_VHFNAV || sPoints[i]->type == WAYPOINT::TYPE_DME)
                    {
                        aSeg.start = sPoints[i];
                        break;
                    }//if
                }//if
                else if(wp.type == sPoints[i]->type)
                {
                    aSeg.start = sPoints[i];
                    break;
                }//else if
                else if(wp.type == -2)
                {
                    aSeg.start = sPoints[i];
                    break;
                }//else if
            }
            if(i == (sPoints.size() - 1) && aSeg.start == NULL && wp.type!=-2)
            {
                //Special case (something is wrong, hence the combination of identifer and type of waypoint in airway is not found in navaids/fixes)
                //We then instead add the waypoint the waypoint first in list that is not XNVU.
                wp.type = -2;
                i = 0;
            }
        }//for

        if(aSeg.start == NULL) continue;

        aSeg.end = NULL;
        sPoints = search(wp2.name, 0, 1, wp2.country);
        for(int i=0; i<sPoints.size(); i++)
        {
            if(sPoints[i]->wpOrigin!=WAYPOINT::ORIGIN_XNVU)
            {
                if(wp2.type == WAYPOINT::TYPE_VHFNAV)
                {
                    if(sPoints[i]->type == WAYPOINT::TYPE_VORDME || sPoints[i]->type == WAYPOINT::TYPE_VOR ||
                       sPoints[i]->type == WAYPOINT::TYPE_TACAN || sPoints[i]->type == WAYPOINT::TYPE_VORTAC ||
                       sPoints[i]->type == WAYPOINT::TYPE_VHFNAV || sPoints[i]->type == WAYPOINT::TYPE_DME)
                    {
                        aSeg.end = sPoints[i];
                        break;
                    }//if
                }//if
                else if(wp2.type == sPoints[i]->type)
                {
                    aSeg.end = sPoints[i];
                    break;
                }//else if
                else if(wp2.type == -2)
                {
                    aSeg.end = sPoints[i];
                    break;
                }//else if
            }

            if(i == (sPoints.size() - 1) && aSeg.start == NULL && wp2.type!=-2)
            {
                //Special case (something is wrong, hence the combination of identifer and type of waypoint in airway is not found in navaids/fixes)
                //We then instead add the waypoint the waypoint first in list that is not XNVU.
                wp2.type = -2;
                i = 0;
            }
        }//for
        if(aSeg.end == NULL) continue;



        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


        //We add the segments to the lASegs list
        //* Note1: Some segments share the same airways, so we duplicate the segment for each airway.

        if(aSeg.direction == 2)
        {
            NVUPOINT* tmp = aSeg.start;
            aSeg.start = aSeg.end;
            aSeg.end = tmp;
        }//if
        else if(aSeg.direction == 0)
        {
            NVUPOINT* tmp = aSeg.start;
            aSeg.start = aSeg.end;
            aSeg.end = tmp;
            for(int i=0; i<sharedAirways.size(); i++)
            {
                AIRWAY_SEGMENT_X11* nASeg = new AIRWAY_SEGMENT_X11();
                nASeg->start = aSeg.start;
                nASeg->end = aSeg.end;
                //nASeg->direction = aSeg.direction;
                nASeg->hilo = aSeg.hilo;
                nASeg->base = aSeg.base;
                nASeg->top = aSeg.top;
                nASeg->airway = sharedAirways.at(i);

                lASegs.push_back(nASeg);
            }
            tmp = aSeg.start;
            aSeg.start = aSeg.end;
            aSeg.end = tmp;
        }//else if

        for(int i=0; i<sharedAirways.size(); i++)
        {
            AIRWAY_SEGMENT_X11* nASeg = new AIRWAY_SEGMENT_X11();
            nASeg->start = aSeg.start;
            nASeg->end = aSeg.end;
            //nASeg->direction = aSeg.direction;
            nASeg->hilo = aSeg.hilo;
            nASeg->base = aSeg.base;
            nASeg->top = aSeg.top;
            nASeg->airway = sharedAirways.at(i);

            lASegs.push_back(nASeg);
        }

    }//while

    //Everthing us loaded into memory, now let us sort everthing out.
    //The list will be sorted after the airway ICAO identifier.
    std::sort(lASegs.begin(), lASegs.end(), AIRWAY_SEGMENT_X11::sort);

    std::vector<AIRWAY_SEGMENT_X11*>::iterator it;
    for(it = lASegs.begin(); it!=lASegs.end(); it++)
    {
        std::pair<std::vector<AIRWAY_SEGMENT_X11*>::iterator, std::vector<AIRWAY_SEGMENT_X11*>::iterator> ret = std::equal_range(lASegs.begin(), lASegs.end(), *it, AIRWAY_SEGMENT_X11::sort);
        std::list<AIRWAY_SEGMENT_X11*> lAW;
        std::list<AIRWAY_SEGMENT_X11*>::iterator ulit;
        std::list<AIRWAY_SEGMENT_X11*>::iterator ulit2;

        for(it=ret.first; it!=ret.second; it++) lAW.push_back(*it);
        if(it!=lASegs.begin()) it--;

        for(ulit = lAW.begin(); ulit!=lAW.end(); ulit++)
        {
            for(ulit2 = std::next(ulit, 1); ulit2!=lAW.end(); ulit2++)
            {
                if(AIRWAY_SEGMENT_X11::isSameIgnoreHILO((*ulit), (*ulit2)))
                {
                    ulit = lAW.erase(ulit);
                    ulit2 = ulit;
                    continue;
                }//if
            }//for
        }//for


        while(lAW.size()>0)
        {
            std::list<AIRWAY_SEGMENT_X11*> lCSeg;	//Sub airway
            std::list<AIRWAY_SEGMENT_X11*>::iterator cit;
            std::list<AIRWAY_SEGMENT_X11*>::iterator lit;

            lCSeg.push_back((*lAW.begin()));
            lAW.erase(lAW.begin());

            lit = lAW.begin();
            while(lit!=lAW.end())
            {

                //for(cit = lCSeg.begin(); cit!=lCSeg.end(); cit++)
                cit = lCSeg.begin();
                while(cit!=lCSeg.end())
                {
                    if((*lit)->end == (*cit)->start && (*lit)->start!=(*cit)->end)
                    {
                        lCSeg.insert(cit, *lit);
                        lAW.erase(lit);
                        lit = lAW.begin();

                        if(lit == lAW.end()) goto while_break;
                        cit = lCSeg.begin();
                        continue;
                    }//if
                    else if((*lit)->start == (*cit)->end && (*lit)->end!=(*cit)->start)
                    {
                        lCSeg.insert(std::next(cit,1), *lit);
                        lAW.erase(lit);
                        lit = lAW.begin();

                        if(lit == lAW.end()) goto while_break;
                        cit = lCSeg.begin();
                        continue;
                    }//else

                    cit++;
                }//while

                lit++;
            }//while
            while_break:

            AIRWAY* ats = new AIRWAY();
            ats->name = (*lCSeg.begin())->airway;
            ats->distance = 0;

            for(cit = lCSeg.begin(); cit!=lCSeg.end(); cit++)
            {
                if(cit == lCSeg.begin())
                {
                    ats->lATS.push_back((*cit)->start);
                }//if
                ats->lATS.push_back((*cit)->end);
                ats->distance+=LMATH::calc_distance((*cit)->start->latlon, (*cit)->end->latlon);
            }//for

            NVUPOINT* wpA = new NVUPOINT();
            wpA->type = WAYPOINT::TYPE_AIRWAY;
            wpA->wpOrigin = _origin;
            wpA->name = ats->name;
            wpA->latlon = ats->lATS[0]->latlon;
            wpA->MD = ats->lATS[0]->MD;
            wpA->data = ats;

            //lAirways.push_back(ats);
            lWP.insert(std::make_pair(wpA->name, wpA));
            __DATA_LOADED[__CURRENT_LOADING]++;
        }//while

        if(it == lASegs.end()) break;
    }//for

    for(int i=0; i<lASegs.size(); i++) delete lASegs.at(i);
    lASegs.clear();
}

void XFMS_DATA::validate_earthnav_XP11(const QStringList &record, int _origin)
{
    if(record.size()<9) return;

    NVUPOINT* wp = new NVUPOINT();

    for(int i=0; i<record.size(); i++)
    {
        QString qstr = record.at(i).simplified();
        int _type;
        switch(i)
        {
            case 0:	//Type
                _type = qstr.toInt();
                if(_type==2 || _type==3 || _type == 4 || _type == 5 || _type == 13) //NDB, VHFNAV, LOCALIZER, ILS, STANDALONE DME
                {
                    wp = new NVUPOINT();
                    wp->type = -_type;
                }//if
                else return;
            break;
            case 1:	//Latitude
                wp->latlon.x = qstr.toDouble();
            break;
            case 2:	//Longitude
                wp->latlon.y = qstr.toDouble();
            break;
            case 3: //Elevation
                wp->elev = qstr.toInt();
            break;
            case 4: //Frequency
                if(wp->type == -2)
                {
                    wp->freq = qstr.toDouble();
                }
                else
                {
                    wp->freq = (qstr.length() == 6 ? qstr.toDouble()/1000.0 : qstr.toDouble()/100.0);
                }
            break;
            case 5: //Range
                wp->range = (int) qstr.toDouble()*1.852;
            break;
            case 6: //Angle Deviation
                wp->ADEV = qstr.toDouble();
            break;
            case 7: //Identifier
                wp->name = qstr;
            break;
            case 8: //TODO: Airport or enroute (ENRT), maybe store it also
                if(wp->type == -4 || wp->type == -5)
                {
                    wp->name2 = qstr;
                }
            break;
            case 9: //Country
                wp->country = qstr;
            break;
            case 10:
                if(wp->type == -4 || wp->type == -5)
                {
                    wp->name2 = wp->name2 + " " + qstr;
                    wp->type = (wp->type == -4 ? WAYPOINT::TYPE_ILS : WAYPOINT::TYPE_LOC);
                    i = record.size();
                    break;
                }

            default: //Real name and VOR/DME type
                //Concat real name, and if VOR/DME is found, we have completed that waypoint
                QString name2;
                if(wp->type == -2)
                {
                    while(i<record.size())
                    {
                        if(qstr.compare("NDB")==0)
                        {
                            wp->type = WAYPOINT::TYPE_NDB;
                            i = record.size();
                            break;
                        }
                        if(name2.isEmpty()) name2 = qstr;
                        else name2 = name2 + " " + qstr;

                        i++;
                        if(!(i<record.size())) break;
                        qstr = record.at(i).simplified();
                    }//while
                }//if
                else if(wp->type == -3)
                {
                    while(i<record.size())
                    {
                        if(qstr.compare("VOR/DME")==0 || qstr.compare("VOR-DME")==0)

                        {
                            wp->type = WAYPOINT::TYPE_VORDME;
                            i = record.size();
                            break;
                        }
                        else if(qstr.compare("VORTAC")==0)
                        {
                            wp->type = WAYPOINT::TYPE_VORTAC;
                            i = record.size();
                            break;
                        }
                        else if(qstr.compare("VOR") == 0)
                        {
                            wp->type = WAYPOINT::TYPE_VOR;
                            i = record.size();
                            break;
                        }
                        else if(qstr.compare("TACAN") == 0)
                        {
                            wp->type = WAYPOINT::TYPE_TACAN;
                            i = record.size();
                            break;
                        }

                        if(name2.isEmpty()) name2 = qstr;
                        else name2 = name2 + " " + qstr;

                        i++;
                        if(!(i<record.size())) break;
                        qstr = record.at(i).simplified();
                    }//while
                }//else if
                else if(wp->type == -13)
                {
                    while(i<record.size())
                    {
                        if(qstr.compare("DME")==0)
                        {
                            wp->type = WAYPOINT::TYPE_DME;
                            i = record.size();
                            break;
                        }
                        if(name2.isEmpty()) name2 = qstr;
                        else name2 = name2 + " " + qstr;

                        i++;
                        if(!(i<record.size())) break;
                        qstr = record.at(i).simplified();
                    }//while
                }//else if
                wp->name2 = name2;

                //If a valid type is not found, do not add this navaid
                if(wp->type<0)
                {
                    delete wp;
                    return;
                }
        }//switch
    }//for
    wp->wpOrigin = _origin;
    wp->MD = calc_magvar(wp->latlon.x, wp->latlon.y, dat, (double(LMATH::feetToMeter(wp->elev))/1000.0));


    lWP.insert(std::make_pair(wp->name, wp));
    lWP2.insert(std::make_pair(wp->name2, wp));
    __DATA_LOADED[__CURRENT_LOADING]++;

    if(wp->type == WAYPOINT::TYPE_VORDME || wp->type == WAYPOINT::TYPE_VORTAC) lXVORDME.push_back(wp);
    /*
    if(wp->type == WAYPOINT::TYPE_NDB) lXNDB.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_VOR) lXVOR.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_VORDME || wp->type == WAYPOINT::TYPE_VORTAC) lXVORDME.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_DME || wp->type == WAYPOINT::TYPE_TACAN) lXDME.push_back(wp);
    */
}

/*
void XFMS_DATA::validate_sidstar_XP11(QFile& _infile, const QString _airport)
{
    while(!infile.atEnd())
    {
        bool bError = false;
        int type;
        QStringList record;
        QString qstr;
        QString line;

        line = infile.readLine();
        record = line.split(',',  QString::SkipEmptyParts);
        try
        {
            for(int i=0; i<record.size(); i++)
            {
                qstr = record[i].simplified();
                switch(i)
                {
                case 0: //Procedure array identifer, i.e. SID:10
                    //wp.name = qstr;
                    break;
                case 1: //Region i.e. ES
                    wp.country = qstr;
                    break;
                case 2: //Type of fix 11=fix, 2=enroute NDB, 3 = VOR, TACAN or DME
                    type = qstr.toInt();
                    if(type == 11) wp.type = WAYPOINT::TYPE_FIX;
                    else if(type == 2) wp.type = WAYPOINT::TYPE_NDB;
                    else if(type == 3) wp.type = WAYPOINT::TYPE_VHFNAV;
                    else bError = true;
                    break;
                case 3: //Fix ICAO
                    wp2.name = qstr;
                    break;
                case 4: //Region i.e. ES
                    wp2.country = qstr;
                    break;
                case 5: //Type of fix 11=fix, 2=enroute NDB, 3 = VOR, TACAN or DME
                    type = qstr.toInt();
                    if(type == 11) wp2.type = WAYPOINT::TYPE_FIX;
                    else if(type == 2) wp2.type = WAYPOINT::TYPE_NDB;
                    else if(type == 3) wp2.type = WAYPOINT::TYPE_VHFNAV;
                    else bError = true;
                    break;
                case 6: //Directional (N, F, B)
                    if(qstr.compare("N", Qt::CaseInsensitive) == 0) aSeg.direction = 0;
                    else if(qstr.compare("F", Qt::CaseInsensitive) == 0) aSeg.direction = 1;
                    else if(qstr.compare("B", Qt::CaseInsensitive) == 0) aSeg.direction = 2;
                    else bError = true;
                    break;
                case 7: //Airway low (1) or high (2)
                    aSeg.hilo = qstr.toInt();
                    break;
                case 8: //Base of airway FL
                    aSeg.base = qstr.toInt();
                    break;
                case 9: //Top of airway FL
                    aSeg.top = qstr.toInt();
                    break;
                case 10:
                    //aSeg.sharedAirways = qstr.split('-', QString::SkipEmptyParts);
                    sharedAirways = qstr.split('-', QString::SkipEmptyParts);
                    break;
                }//switch
            }//for
        }//try
        catch(...)
        {
            continue;
        }//catch

        if(bError) continue;
    }//while
}
*/

//KLN90B
/*
void XFMS_DATA::validate_airport_KLN90B(const QStringList& record)
{
    if(record.size()<5) return;

    NVUPOINT* wp;
    for(int i=0; i<record.size(); i++)
    {
        QString qstr = record.at(i).simplified();
        switch(i)
        {
            case 0:	//Runway or airport
                if(qstr.compare("A")!=0) return;
                wp = new NVUPOINT();
                wp->type = WAYPOINT::TYPE_AIRPORT;
            break;
            case 1:	//ICAO
                wp->name = qstr;
            break;
            case 2: //Real name
                wp->name2 = qstr;
            break;
            case 3:	//Latitude
                wp->latlon.x = qstr.toInt()*0.000001;
            break;
            case 4:	//Longitude
                wp->latlon.y = qstr.toInt()*0.000001;
            break;
            case 5: //Elevation
                wp->elev = qstr.toInt();
            break;
        }//switch
    }//for

    wp->wpOrigin = WAYPOINT::ORIGIN_AIRAC_AIRPORTS;
    wp->MD = calc_magvar(wp->latlon.x, wp->latlon.y, dat, (double(LMATH::feetToMeter(wp->elev))/1000.0));
    lWP.insert(std::make_pair(wp->name, wp));
    lWP2.insert(std::make_pair(wp->name2, wp));
    lAirports.push_back(wp);
}

void XFMS_DATA::validate_navaid_KLN90B(const QStringList &record)
{
    if(record.size()<9) return;

    NVUPOINT* wp = new NVUPOINT();
    bool isILS;

    for(int i=0; i<record.size(); i++)
    {
        QString qstr = record.at(i).simplified();
        switch(i)
        {
            case 0:	//Identifier
                wp->name = qstr;
            break;
            case 1: //Real name
                wp->name2 = qstr;
                if(wp->name2.contains(" ILS/")) isILS = true;
                else isILS = false;
            break;
            case 2: //Frequency
                wp->freq =qstr.toDouble();
            break;
            case 3: //Is VOR?
                if(isILS) wp->type = WAYPOINT::TYPE_ILS;
                else wp->type = (qstr.toInt()) ? WAYPOINT::TYPE_VOR : 0;
            break;
            case 4:	//Is DME?
                if(!isILS)
                {
                    if(qstr.toInt())
                    {
                        wp->type = (wp->type) ? WAYPOINT::TYPE_VORDME : WAYPOINT::TYPE_DME;  //Is VORDME or DME
                    }
                    else if(!wp->type) wp->type = WAYPOINT::TYPE_NDB;   //Is NDB
                }
            break;
            case 5: //Range
                wp->range = qstr.toInt();
            break;
            case 6:	//Latitude
                wp->latlon.x = qstr.toInt()*0.000001;
            break;
            case 7:	//Longitude
                wp->latlon.y = qstr.toInt()*0.000001;
            break;
            case 8: //Elevation
                wp->elev = qstr.toInt();
            break;
            case 9: //Country code
                wp->country = qstr;
            break;
        }//switch
    }//for

    wp->wpOrigin = WAYPOINT::ORIGIN_AIRAC_NAVAIDS;
    wp->MD = calc_magvar(wp->latlon.x, wp->latlon.y, dat, (double(LMATH::feetToMeter(wp->elev))/1000.0));
    lWP.insert(std::make_pair(wp->name, wp));
    lWP2.insert(std::make_pair(wp->name2, wp));

    if(wp->type == WAYPOINT::TYPE_NDB)    lNDB.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_VOR)    lVOR.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_DME)    lDME.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_VORDME) lVORDME.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_ILS) lILS.push_back(wp);
}

void XFMS_DATA::validate_waypoint_KLN90B(const QStringList& record)
{
    if(record.size()<3) return;

    NVUPOINT* wp = new NVUPOINT();
    wp->type = WAYPOINT::TYPE_FIX;

    for(int i=0; i<record.size(); i++)
    {
        QString qstr = record.at(i).simplified();
        switch(i)
        {
            case 0:	//Identifier
                wp->name = qstr;
            break;
            case 1:	//Latitude
                wp->latlon.x = qstr.toInt()*0.000001;
            break;
            case 2:	//Longitude
                wp->latlon.y = qstr.toInt()*0.000001;
            break;
            case 3: //Country code
                wp->country = qstr;
            break;
        }//switch
    }//for

    wp->wpOrigin = WAYPOINT::ORIGIN_AIRAC_WAYPOINTS;
    wp->MD = calc_magvar(wp->latlon.x, wp->latlon.y, dat);

    double d;
    NVUPOINT* wpSimilar = getClosestWaypointType(wp, d);
    if(wpSimilar && d<=DEFAULT_WAYPOINT_MARGIN)
    {
        if(wpSimilar->wpOrigin!=WAYPOINT::ORIGIN_XNVU)
        {
            delete wp;
            return;
        }
    }

    lWP.insert(std::make_pair(wp->name, wp));
    lWP2.insert(std::make_pair(wp->name2, wp));
    lFixes.push_back(wp);
}

void XFMS_DATA::validate_airways_KLN90B(QFile& infile)
{
    QString qstr;
    NVUPOINT wpA, wpB;
    AIRWAY* ats = NULL;
    int inboundCourse = 0;
    int outboundCourse = 0;
    double distance = 0;

    while_loop:
    while(!infile.atEnd())
    {
        QString line = infile.readLine();
        QStringList record;
        record = line.split('|',  QString::SkipEmptyParts);

        for(int i=0; i<record.size(); i++)
        {
            qstr = record.at(i).simplified();
            switch(i)
            {
                case 0:	//Identifier
                    if(qstr.compare("A") == 0)  //If beginning of a ATS route, get the ATS name and continue on next line.
                    {
                        if(ats)
                        {
                            if(ats->lATS.size()>0)  //If ats has waypoints, this is the end of this ATS route.
                            {
                                NVUPOINT* wpA = new NVUPOINT;
                                wpA->type = WAYPOINT::TYPE_AIRWAY;
                                wpA->wpOrigin = WAYPOINT::WAYPOINT::ORIGIN_AIRAC_ATS;
                                wpA->name = ats->name;
                                wpA->latlon = ats->lATS[0]->latlon;
                                wpA->MD = ats->lATS[0]->MD;
                                wpA->data = ats;
                                ats->distance = 0;
                                for(int k=0; k<ats->lATS.size()-1; k++)
                                {
                                    ats->distance+=LMATH::calc_distance(ats->lATS[k]->latlon, ats->lATS[k+1]->latlon);
                                }
                                lAirways.push_back(ats);
                                lWP.insert(std::make_pair(wpA->name, wpA));
                            }//if
                            else delete ats;
                        }//if
                        ats = new AIRWAY();
                        ats->name = record.at(1).simplified();
                        goto while_loop;
                    }
                    else if(qstr.compare("S")!=0) goto while_loop; //If not A, it should be S, otherwise the line is corrupt
                break;
                case 1:	//Waypoint A identifier
                    wpA.name = qstr;
                break;
                case 2:	//A Latitude
                    wpA.latlon.x = qstr.toInt()*0.000001;
                break;
                case 3:	//A Longitude
                    wpA.latlon.y = qstr.toInt()*0.000001;
                break;
                case 4:	//Waypoint B identifier
                    wpB.name = qstr;
                break;
                case 5:	//B Latitude
                    wpB.latlon.x = qstr.toInt()*0.000001;
                break;
                case 6:	//B Longitude
                    wpB.latlon.y = qstr.toInt()*0.000001;
                break;
                case 7: //Inbound course
                    inboundCourse = qstr.toInt();
                break;
                case 8: //Outbound course
                    outboundCourse = qstr.toInt();
                break;
                case 9: //Distance
                    distance = qstr.toInt()*0.01;
                break;
            }//switch
        }//for

        wpA.wpOrigin = WAYPOINT::ORIGIN_AIRAC_ATS;
        wpA.MD =  calc_magvar(wpA.latlon.x, wpA.latlon.y, dat);
        wpA.type = WAYPOINT::TYPE_FIX;
        wpB.wpOrigin = WAYPOINT::ORIGIN_AIRAC_ATS;
        wpB.MD =  calc_magvar(wpB.latlon.x, wpB.latlon.y, dat);
        wpB.type = WAYPOINT::TYPE_FIX;

        NVUPOINT* a = NULL;
        NVUPOINT* b = NULL;


        if(DialogSettings::distAlignATS)
        {
            double d;

            a = getClosestWaypointType(&wpA, d);
            if(a) if(a->type!=wpA.type || a->wpOrigin==WAYPOINT::ORIGIN_XNVU || d>DialogSettings::distAlignMargin) a = NULL;

            b = getClosestWaypointType(&wpB, d);
            if(b) if(b->type!=wpB.type || b->wpOrigin==WAYPOINT::ORIGIN_XNVU || d>DialogSettings::distAlignMargin) b = NULL;
        }//if
        else
        {
            double d;
            a = getClosestWaypointType(&wpA, d);
            if(a) if(a->type!=wpA.type || a->wpOrigin==WAYPOINT::ORIGIN_XNVU || d>DEFAULT_WAYPOINT_MARGIN) a = NULL;

            b = getClosestWaypointType(&wpB, d);
            if(b) if(b->type!=wpB.type || b->wpOrigin==WAYPOINT::ORIGIN_XNVU || d>DEFAULT_WAYPOINT_MARGIN) b = NULL;
        }

        if(ats->lATS.size()==0)
        {
            if(!a)
            {
                a = new NVUPOINT(wpA);
                lWP.insert(std::make_pair(a->name, a));
            }//if

            if(!b)
            {
                b = new NVUPOINT(wpB);
                lWP.insert(std::make_pair(b->name, b));
            }//if
            ats->lATS.push_back(a);
            ats->lATS.push_back(b);
        }
        else
        {
            if(!b)
            {
                b = new NVUPOINT(wpB);
                lWP.insert(std::make_pair(b->name, b));
            }//if
            ats->lATS.push_back(b);
        }//else
    }//while
}
*/

void XFMS_DATA::validate_airport_XP10(const QStringList& record)
{
    if(record.size()<5) return;

    NVUPOINT* wp;
	for(int i=0; i<record.size(); i++)
	{
        QString qstr = record.at(i).simplified();
        switch(i)
		{
			case 0:	//Runway or airport
                if(qstr.compare("A")!=0) return;
                wp = new NVUPOINT();
                wp->type = WAYPOINT::TYPE_AIRPORT;
			break;
			case 1:	//ICAO
                wp->name = qstr;
			break;
			case 2: //Real name
                wp->name2 = qstr;
			break;
			case 3:	//Latitude
                wp->latlon.x = qstr.toDouble();
			break;
			case 4:	//Longitude
                wp->latlon.y = qstr.toDouble();
			break;
			case 5: //Elevation
                wp->elev = qstr.toInt();
			break;
			case 6: //Transition altitude
                wp->trans_alt = qstr.toInt();
			break;
			case 7:
                wp->trans_level = qstr.toInt();
			break;
			case 8:
                wp->length = qstr.toInt();
			break;
		}//switch
	}//for

    wp->wpOrigin = WAYPOINT::ORIGIN_X10_AIRAC_AIRPORTS;
    wp->MD = calc_magvar(wp->latlon.x, wp->latlon.y, dat, (double(LMATH::feetToMeter(wp->elev))/1000.0));
    lWP.insert(std::make_pair(wp->name, wp));
    lWP2.insert(std::make_pair(wp->name2, wp));
    __DATA_LOADED[__CURRENT_LOADING]++;
    //lAirports.push_back(wp);
}

void XFMS_DATA::validate_navaid_XP10(const QStringList &record)
{
	if(record.size()<9) return;

    NVUPOINT* wp = new NVUPOINT();
    bool isILS;

	for(int i=0; i<record.size(); i++)
	{
        QString qstr = record.at(i).simplified();
        switch(i)
		{
			case 0:	//Identifier
                wp->name = qstr;
			break;
			case 1: //Real name
                wp->name2 = qstr;
                if(wp->name2.contains(" ILS/")) isILS = true;
                else isILS = false;
			break;
			case 2: //Frequency
                wp->freq =qstr.toDouble();
			break;
            case 3: //Is VOR?
                if(isILS) wp->type = WAYPOINT::TYPE_ILS;
                else wp->type = (qstr.toInt()) ? WAYPOINT::TYPE_VOR : 0;
			break;
            case 4:	//Is DME?
                if(!isILS)
                {
                    if(qstr.toInt())
                    {
                        wp->type = (wp->type) ? WAYPOINT::TYPE_VORDME : WAYPOINT::TYPE_DME;  //Is VORDME or DME
                    }
                    else if(!wp->type) wp->type = WAYPOINT::TYPE_NDB;   //Is NDB
                }
			break;
			case 5: //Range
                wp->range = qstr.toInt();
			break;
			case 6:	//Latitude
                wp->latlon.x = qstr.toDouble();
			break;
			case 7:	//Longitude
                wp->latlon.y = qstr.toDouble();
			break;
			case 8: //Elevation
                wp->elev = qstr.toInt();
			break;
			case 9: //Country code
                wp->country = qstr;
			break;
		}//switch
	}//for

    wp->wpOrigin = WAYPOINT::ORIGIN_X10_AIRAC_NAVAIDS;
    wp->MD = calc_magvar(wp->latlon.x, wp->latlon.y, dat, (double(LMATH::feetToMeter(wp->elev))/1000.0));
    lWP.insert(std::make_pair(wp->name, wp));
    lWP2.insert(std::make_pair(wp->name2, wp));
    __DATA_LOADED[__CURRENT_LOADING]++;


    /*
    if(wp->type == WAYPOINT::TYPE_NDB)    lNDB.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_VOR)    lVOR.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_DME)    lDME.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_VORDME) lVORDME.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_ILS) lILS.push_back(wp);
    */
}

void XFMS_DATA::validate_earthnav_XP10(const QStringList &record)
{
    if(record.size()<9) return;

    NVUPOINT* wp = new NVUPOINT();

    for(int i=0; i<record.size(); i++)
    {
        QString qstr = record.at(i).simplified();
        switch(i)
        {
            case 0:	//Type
                wp->type = qstr.toInt();
                if(wp->type!=2 && wp->type!=3 && wp->type!=13)
                {
                    delete wp;
                    return;
                }
            break;
            case 1:	//Latitude
                wp->latlon.x = qstr.toDouble();
            break;
            case 2:	//Longitude
                wp->latlon.y = qstr.toDouble();
            break;
            case 3: //Elevation
                wp->elev = qstr.toInt();
            break;
            case 4: //Frequency
                if(wp->type == 2)
                {
                    wp->freq = qstr.toDouble();
                }
                else
                {
                    wp->freq = (qstr.length() == 6 ? qstr.toDouble()/1000.0 : qstr.toDouble()/100.0);
                }
            break;
            case 5: //Range
                wp->range = (int) qstr.toDouble()*1.852;
            break;
            case 6: //Angle Deviation
                wp->ADEV = qstr.toDouble();
            break;
            case 7: //Identifier
                wp->name = qstr;
            break;
            default: //Real name and VOR/DME type
                //Concat real name, and if VOR/DME is found, we have completed that waypoint
                QString name2;
                if(wp->type == 2)
                {
                    while(i<record.size())
                    {
                        if(qstr.compare("NDB")==0)
                        {
                            wp->type = WAYPOINT::TYPE_NDB;
                            i = record.size();
                            break;
                        }
                        if(name2.isEmpty()) name2 = qstr;
                        else name2 = name2 + " " + qstr;

                        i++;
                        if(!(i<record.size())) break;
                        qstr = record.at(i).simplified();
                    }//while
                }//if
                else if(wp->type == 3)
                {
                    while(i<record.size())
                    {
                        if(qstr.compare("VOR/DME")==0 || qstr.compare("VORTAC")==0)
                        {
                            wp->type = WAYPOINT::TYPE_VORDME;
                            i = record.size();
                            break;
                        }
                        if(name2.isEmpty()) name2 = qstr;
                        else name2 = name2 + " " + qstr;

                        i++;
                        if(!(i<record.size())) break;
                        qstr = record.at(i).simplified();
                    }//while
                }//else if
                else if(wp->type == 13)
                {
                    while(i<record.size())
                    {
                        if(qstr.compare("DME")==0 || qstr.compare("NDB/DME")==0 || qstr.compare("TACAN")==0)
                        {
                            wp->type = WAYPOINT::TYPE_DME;
                            i = record.size();
                            break;
                        }
                        if(name2.isEmpty()) name2 = qstr;
                        else name2 = name2 + " " + qstr;

                        i++;
                        if(!(i<record.size())) break;
                        qstr = record.at(i).simplified();
                    }//while
                }//else if
                wp->name2 = name2;

                //If not VOR/DME is found, do not add this navaid
                if(wp->type != WAYPOINT::TYPE_NDB && wp->type != WAYPOINT::TYPE_VORDME && wp->type != WAYPOINT::TYPE_DME)
                {
                    delete wp;
                    return;
                }
        }//switch
    }//for
    wp->wpOrigin = WAYPOINT::ORIGIN_X10_EARTHNAV;
    wp->MD = calc_magvar(wp->latlon.x, wp->latlon.y, dat, (double(LMATH::feetToMeter(wp->elev))/1000.0));


    lWP.insert(std::make_pair(wp->name, wp));
    lWP2.insert(std::make_pair(wp->name2, wp));
    __DATA_LOADED[__CURRENT_LOADING]++;

    if(wp->type == WAYPOINT::TYPE_VORDME) lXVORDME.push_back(wp);

    /*
    if(wp->type == WAYPOINT::TYPE_NDB) lXNDB.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_VORDME) lXVORDME.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_DME) lXDME.push_back(wp);
    */
}

void XFMS_DATA::validate_waypoint_XP10(const QStringList& record)
{
    if(record.size()<3) return;

    NVUPOINT* wp = new NVUPOINT();
    wp->type = WAYPOINT::TYPE_FIX;

	for(int i=0; i<record.size(); i++)
	{
        QString qstr = record.at(i).simplified();
        switch(i)
		{
			case 0:	//Identifier
                wp->name = qstr;
			break;
			case 1:	//Latitude
                wp->latlon.x = qstr.toDouble();
			break;
			case 2:	//Longitude
                wp->latlon.y = qstr.toDouble();
			break;
			case 3: //Country code
                wp->country = qstr;
			break;
		}//switch
	}//for

    wp->wpOrigin = WAYPOINT::ORIGIN_X10_AIRAC_FIXES;
    wp->MD = calc_magvar(wp->latlon.x, wp->latlon.y, dat);

    //double d;
    //NVUPOINT* wpSimilar = getClosestWaypointType(wp->latlon, wp->name, 0, d);
    bool isSimilarFound = false;
    std::vector< std::pair<NVUPOINT*, double> > wpSimilar = getClosestWaypointType(wp->latlon, wp->name, WAYPOINT::TYPE_FIX);
    for(unsigned int i= 0; i<wpSimilar.size(); i++)
    {
        if(wpSimilar[i].second<=DEFAULT_WAYPOINT_MARGIN && wpSimilar[i].first->wpOrigin!=WAYPOINT::ORIGIN_XNVU)
        {
            isSimilarFound = true;
            break;
        }
    }//for
    if(isSimilarFound)
    {
        delete wp;
        return;
    }


    lWP.insert(std::make_pair(wp->name, wp));
    lWP2.insert(std::make_pair(wp->name2, wp));
    __DATA_LOADED[__CURRENT_LOADING]++;
    //lFixes.push_back(wp);
}

void XFMS_DATA::validate_airways_XP10(QFile& infile)
{
    QString qstr;
    NVUPOINT wpA, wpB;
    AIRWAY* ats = NULL;
    int inboundCourse = 0;
    int outboundCourse = 0;
    double distance = 0;

    while_loop:
    while(!infile.atEnd())
    {
        QString line = infile.readLine();
        QStringList record;
        record = line.split(',',  QString::SkipEmptyParts);

        for(int i=0; i<record.size(); i++)
        {
            qstr = record.at(i).simplified();
            switch(i)
            {
                case 0:	//Identifier
                    if(qstr.compare("A") == 0)  //If beginning of a ATS route, get the ATS name and continue on next line.
                    {
                        if(ats)
                        {
                            if(ats->lATS.size()>0)  //If ats has waypoints, this is the end of this ATS route.
                            {
                                NVUPOINT* wpA = new NVUPOINT;
                                wpA->type = WAYPOINT::TYPE_AIRWAY;
                                wpA->wpOrigin = WAYPOINT::WAYPOINT::ORIGIN_X10_AIRAC_ATS;
                                wpA->name = ats->name;
                                wpA->latlon = ats->lATS[0]->latlon;
                                wpA->MD = ats->lATS[0]->MD;
                                wpA->data = ats;
                                ats->distance = 0;
                                for(int k=0; k<ats->lATS.size()-1; k++)
                                {
                                    ats->distance+=LMATH::calc_distance(ats->lATS[k]->latlon, ats->lATS[k+1]->latlon);
                                }
                                //lAirways.push_back(ats);
                                lWP.insert(std::make_pair(wpA->name, wpA));
                                __DATA_LOADED[__CURRENT_LOADING]++;
                            }//if
                            else delete ats;
                        }//if
                        ats = new AIRWAY();
                        ats->name = record.at(1).simplified();
                        goto while_loop;
                    }
                    else if(qstr.compare("S")!=0) goto while_loop; //If not A, it should be S, otherwise the line is corrupt
                break;
                case 1:	//Waypoint A identifier
                    wpA.name = qstr;
                break;
                case 2:	//A Latitude
                    wpA.latlon.x = qstr.toDouble();
                break;
                case 3:	//A Longitude
                    wpA.latlon.y = qstr.toDouble();
                break;
                case 4:	//Waypoint B identifier
                    wpB.name = qstr;
                break;
                case 5:	//B Latitude
                    wpB.latlon.x = qstr.toDouble();
                break;
                case 6:	//B Longitude
                    wpB.latlon.y = qstr.toDouble();
                break;
                case 7: //Inbound course
                    inboundCourse = qstr.toInt();
                break;
                case 8: //Outbound course
                    outboundCourse = qstr.toInt();
                break;
                case 9: //Distance
                    distance = qstr.toDouble();
                break;
            }//switch
        }//for

        wpA.wpOrigin = WAYPOINT::ORIGIN_X10_AIRAC_ATS;
        wpA.MD =  calc_magvar(wpA.latlon.x, wpA.latlon.y, dat);
        wpA.type = WAYPOINT::TYPE_FIX;
        wpB.wpOrigin = WAYPOINT::ORIGIN_X10_AIRAC_ATS;
        wpB.MD =  calc_magvar(wpB.latlon.x, wpB.latlon.y, dat);
        wpB.type = WAYPOINT::TYPE_FIX;

        NVUPOINT* a = NULL;
        NVUPOINT* b = NULL;


        double d;
        std::vector< std::pair<NVUPOINT*, double> > cL = getClosestWaypointType(wpA.latlon, wpA.name, 0);
        for(unsigned int i=0; i<cL.size(); i++) if(cL[i].first->wpOrigin!=WAYPOINT::ORIGIN_XNVU && cL[i].second<=DEFAULT_WAYPOINT_MARGIN){a = cL[i].first; break;};
        cL = getClosestWaypointType(wpB.latlon, wpA.name, 0);
        for(unsigned int i=0; i<cL.size(); i++) if(cL[i].first->wpOrigin!=WAYPOINT::ORIGIN_XNVU && cL[i].second<=DEFAULT_WAYPOINT_MARGIN){b = cL[i].first; break;};

        /*
        a = getClosestWaypointType(wpA.latlon, wpA.name, 0, d);
        if(a) if(a->type!=wpA.type || a->wpOrigin==WAYPOINT::ORIGIN_XNVU || d>DEFAULT_WAYPOINT_MARGIN) a = NULL;

        b = getClosestWaypointType(wpB.latlon, wpB.name, 0, d);
        if(b) if(b->type!=wpB.type || b->wpOrigin==WAYPOINT::ORIGIN_XNVU || d>DEFAULT_WAYPOINT_MARGIN) b = NULL;
        */

        if(ats->lATS.size()==0)
        {
            if(!a)
            {
                a = new NVUPOINT(wpA);
                lWP.insert(std::make_pair(a->name, a));
            }//if

            if(!b)
            {
                b = new NVUPOINT(wpB);
                lWP.insert(std::make_pair(b->name, b));
            }//if
            ats->lATS.push_back(a);
            ats->lATS.push_back(b);
        }
        else
        {
            if(!b)
            {
                b = new NVUPOINT(wpB);
                lWP.insert(std::make_pair(b->name, b));
            }//if
            ats->lATS.push_back(b);
        }//else
    }//while
}
//wpRef: reference waypoint (If more than 1 waypoints of the the same identifier is found, use wpRef to select the closest one)
//rPoint: allocated waypoint returned, or NULL if not valid.
//record: string of custom point
//Returns description of error.
QString XFMS_DATA::validate_custom_point(const NVUPOINT* wpRef, NVUPOINT*& rPoint, const QString& record)
{
    //Validates a point of either a 7-11 string coordinate,
    //or a bearing and distance from an existing waypoint, and then creates a tempory waypoint.
    //Four format examples: 46N078W, 4620N07805W, 562530N0165259E, BEDLA245052
    //Lat/lon format is Degrees, Minutes and Seconds.
    //Note: Waypoints which ends with numbers will not currently work with added bearing and distance,
    //as we do clip the last numbers of string.

    rPoint = NULL;

    //Check if waypoint is detected as bearing and distance from waypoint
    QString sID = record;
    QString sBD = record;
    int lIndex = sID.lastIndexOf(QRegExp("[A-Z]"));
    sID.truncate(lIndex+1);
    sBD.remove(0, lIndex+1);


    if(!sID.isEmpty() && !sBD.isEmpty())
    {
        //We need a reference if waypoint is a bearing/distance waypoint.
        if(wpRef == NULL) return "A waypoint of reference is needed for custom waypoint [" + record + "].";

        //Bearing and distance section needs to be 6 characters
        if(sBD.size() != 6) return "Bearing and distance should be of 6 digits in custom waypoint [" + record + "].";

        //Bearing and distance needs to be a number
        bool isOk;
        sBD.toUInt(&isOk);
        if(!isOk) return "Bearing and distance should be a valid number in custom waypoint [" + record + "].";

        std::vector<NVUPOINT*> lWP = XFMS_DATA::search(sID);
        //Identifier needs to be found in database
        if(lWP.size() == 0) return "Identifier not found in custom waypoint [" + record + "].";

        NVUPOINT* cWP;
        double dMin = std::numeric_limits<double>::max();
        double dCurrent = 0;
        for(int k=0; k<lWP.size(); k++)
        {
            dCurrent = LMATH::calc_distance(wpRef->latlon, lWP[k]->latlon);
            if(dCurrent<dMin)
            {
                dMin = dCurrent;
                cWP = lWP[k];
            }
        }

        //cWP is now the base point, calculate now the position of bearing and distance
        int brng = sBD.left(3).toInt();
        int dist = sBD.right(3).toInt();

        rPoint = new NVUPOINT();
        LMATH::calc_destination_orthodromic(cWP->latlon, brng, double(dist)*1.852, rPoint->latlon);

        rPoint->name = record;
        rPoint->type = WAYPOINT::TYPE_LATLON;
        rPoint->MD = calc_magvar(rPoint->latlon.x, rPoint->latlon.y, dat);
        rPoint->wpOrigin = WAYPOINT::ORIGIN_FLIGHTPLAN;

        return NULL;
    }

    //Check if is detected as latitude/longitude
    bool isNE, isOK;
    double lat, lon;
    int format;

    //Search for N/S tag, if not found return.
    lIndex = record.lastIndexOf(QRegExp("[NS]"));
    sID = record.left(lIndex+1);
    if(sID.size()==0) return "Tag N or S is not found in custom waypoint [" + record + "].";

    //Check if tag is N or S
    if(sID[sID.size()-1] == 'N') isNE = true;
    else if(sID[sID.size()-1] == 'S') isNE = false;
    else return "Tag of N or S is not found in custom waypoint [" + record + "].";
    sID.remove(sID.size()-1, 1);

    //Check if coordinate before the tag is a number
    sID.toUInt(&isOK);
    if(!isOK) return "Coordinate should be number in custom waypoint [" + record + "].";

    format = sID.size();
    switch(format)
    {
        case 2: //2 Digit lat (i.e. 46N)
            lat = sID.toInt();
        break;

        case 4: //4 Digit lat (i.e. 4659N
            lat = sID.left(2).toInt();
            sID.remove(0, 2);
            lat = lat + sID.toInt()/60.0;
        break;

        case 6: //6 Digit lat (i.e. 465923N)
            lat = sID.left(2).toInt();
            sID.remove(0, 2);
            lat = lat + sID.left(2).toInt()/60.0;
            sID.remove(0, 2);
            lat = lat + sID.toInt()/3600.0;
        break;
        default:
            //Not a correct format, should be 2, 4 or 6 digits.
            return "Coordinate should have 2, 4 or 6 digits in custom waypoint [" + record + "].";
    }
    if(!isNE) lat = -lat;

    //Search for E/W tag, if not found return.
    sID = record;
    lIndex = sID.lastIndexOf(QRegExp("[NS]"));
    sID = sID.remove(0, lIndex+1);
    lIndex = sID.lastIndexOf(QRegExp("[EW]"));
    sID = sID.left(lIndex+1);
    if(sID.size()==0) return "Tag E or W is not found in custom waypoint [" + record + "].";

    //Check if tag is E or W
    if(sID[sID.size()-1] == 'E') isNE = true;
    else if(sID[sID.size()-1] == 'W') isNE = false;
    else return "Tag E or W is not found in custom waypoint [" + record + "].";
    sID.remove(sID.size()-1, 1);

    //Check if coordinate before the tag is a number
    sID.toUInt(&isOK);
    if(!isOK) return "Coordinate should be number in custom waypoint [" + record + "].";

    format = sID.size();
    switch(format)
    {
        case 3: //3 Digit lon (i.e. 046E)
            lon = sID.toInt();
        break;

        case 5: //5 Digit lon (i.e. 04659E)
            lon = sID.left(3).toInt();
            sID.remove(0, 3);
            lon = lon + sID.toInt()/60.0;
        break;

        case 7: //7 Digit lon (i.e. 0465923E)
            lon = sID.left(3).toInt();
            sID.remove(0, 3);
            lon = lon + sID.left(2).toInt()/60.0;
            sID.remove(0, 2);
            lon = lon + sID.toInt()/3600.0;
        break;
        default:
            //Not a correct format, should be 2, 4 or 6 digits.
            return "Coordinate should have 2, 4 or 6 digits in custom waypoint [" + record + "].";
    }
    if(!isNE) lon = -lon;

    rPoint = new NVUPOINT();
    rPoint->latlon.x = lat;
    rPoint->latlon.y = lon;
    rPoint->name = record;
    rPoint->type = WAYPOINT::TYPE_LATLON;
    rPoint->MD = calc_magvar(rPoint->latlon.x, rPoint->latlon.y, dat);
    rPoint->wpOrigin = WAYPOINT::ORIGIN_FLIGHTPLAN;

    return NULL;
}

void XFMS_DATA::validate_RSBN(const QStringList &record)
{
    if(record.size()<7) return;

    NVUPOINT* wp = new NVUPOINT();
    wp->type = WAYPOINT::TYPE_RSBN;

    for(int i=0; i<record.size(); i++)
    {
        QString qstr = record.at(i).simplified();

        switch(i)
        {
            case 0:	//Channel
                wp->freq = qstr.toInt();
            break;
            case 1:	//Full name
                wp->name2 = qstr;
            break;
            case 2:	//Identifier
                wp->name = qstr;
            break;
            case 3: //VOR frequency????
                //wp->freq = qstr.toDouble();
            break;
            case 4:	//Latitude
                wp->latlon.x = qstr.toDouble();
            break;
            case 5:	//Longitude
                wp->latlon.y = qstr.toDouble();
            break;
            case 6:	//???????
                //wp->longest_runway = qstr.toInt();
            break;

        }//switch
    }//for

    wp->wpOrigin = WAYPOINT::ORIGIN_RSBN;
    wp->MD = calc_magvar(wp->latlon.x, wp->latlon.y, dat);
    lWP.insert(std::make_pair(wp->name, wp));
    lWP2.insert(std::make_pair(wp->name2, wp));
    lRSBN.push_back(wp);
    __DATA_LOADED[__CURRENT_LOADING]++;
}

void XFMS_DATA::validate_xnvu(const QStringList& RAW)
{
    if(RAW.size()<13) return;

    NVUPOINT wp;

    for(int i=0; i<RAW.size(); i++)
    {
        QString qstr = RAW.at(i).simplified();
        switch(i)
        {
            case 0: //TYPE
                wp.type = qstr.toInt();
            break;
            case 1: //Identifier
                wp.name = qstr;
            break;
            case 2: //Name
                wp.name2 = qstr;
            break;
            case 3: //Country
                wp.country = qstr;
            break;
            case 4: //Lat
                wp.latlon.x = qstr.toDouble();
            break;
            case 5: //Long
                wp.latlon.y = qstr.toDouble();
            break;
            case 6: //Elev
                wp.elev = qstr.toInt();
            break;
            case 7: //Freq
                wp.freq = qstr.toDouble();
            break;
            case 8: //Range
                wp.range = qstr.toInt();
            break;
            case 9: //Angle Deviation
                wp.ADEV = qstr.toDouble();
            break;
            case 10: //Transition Alt
                wp.trans_alt = qstr.toInt();
            break;
            case 11: //Tansition Level
                wp.trans_level = qstr.toInt();
            break;
            case 12: //Longest Runway
                wp.length = qstr.toInt();
            break;
        }//Switch
    }//for
    wp.wpOrigin = WAYPOINT::ORIGIN_XNVU;
    wp.MD = calc_magvar(wp.latlon.x, wp.latlon.y, dat, (double(LMATH::feetToMeter(wp.elev))/1000.0));

    NVUPOINT* nwp = new NVUPOINT(wp);

    lWP.insert(std::make_pair(nwp->name, nwp));
    lWP2.insert(std::make_pair(nwp->name2, nwp));
    lXNVU.push_back(nwp);
    __DATA_LOADED[__CURRENT_LOADING]++;
}

int XFMS_DATA::saveXNVUData()
{
    //Type | ID | NAME | COUNTRY | LAT | LON | ELEV | FREQ | RANGE | AD | TALT | TLVL | LRWY
    QFile outfile(XNVU_WPS_FILENAME);
    if(!outfile.open(QIODevice::WriteOnly | QIODevice::Text)) return 0;


    QTextStream out(&outfile);

    for(int i=0; i<lXNVU.size(); i++)
    {
        NVUPOINT* p = lXNVU[i];
        out << qSetRealNumberPrecision(16)
            << p->type << '|' << p->name << '|' << p->name2 << '|' << p->country << '|' << p->latlon.x << '|' << p->latlon.y << '|'
            << p->elev << '|' << p->freq << '|' << p->range << '|' << p->ADEV << '|'
            << p->trans_alt << '|' << p->trans_level << '|' << p->length << "\n";
    }

    outfile.close();
    return 1;
}

/*
int XFMS_DATA::saveXNVUDataTest(const char* _filename)
{
    //Type | ID | NAME | COUNTRY | LAT | LON | ELEV | FREQ | RANGE | AD | TALT | TLVL | LRWY

    std::multimap<std::string, NVUPOINT*>::iterator it = lWP.begin();
    QFile outfile(_filename);
    if(!outfile.open(QIODevice::WriteOnly | QIODevice::Text)) return 0;

    QTextStream out(&outfile);

    for(; it!=lWP.end(); it++)
    {
        NVUPOINT* p = (*it).second;
        out << qSetRealNumberPrecision(16)
            << p->type << '|' << p->name << '|' << p->name2 << '|' << p->country << '|' << p->latlon.x << '|' << p->latlon.y << '|'
            << p->elev << '|' << p->freq << '|' << p->range << '|' << p->ADEV << '|'
            << p->trans_alt << '|' << p->trans_level << '|' << p->longest_runway << "\n";
    }

    outfile.close();
    return 1;
}
*/

int XFMS_DATA::saveXNVUFlightplan(const QString& file, std::vector<NVUPOINT*> lN)
{
    //Type | ID | NAME | COUNTRY | LAT | LON | ALT | ELEV | FREQ | RANGE | AD | TALT | TLVL | LRWY | RSBN_ORIGIN | RSBN_ID | RSBN_NAME | RSBN_FREQ | RSBN_LAT | RSBN_LON
    QFile outfile(file);
    if(!outfile.open(QIODevice::WriteOnly | QIODevice::Text)) return 0;

    QTextStream out(&outfile);

    for(int i=0; i<lN.size(); i++)
    {
        NVUPOINT* p = lN[i];
        out << qSetRealNumberPrecision(16)
            << p->type << '|' << p->name << '|' << p->name2 << '|' << p->country << '|' << p->latlon.x << '|' << p->latlon.y << '|' << p->alt << '|'
            << p->elev << '|' << p->freq << '|' << p->range << '|' << p->ADEV << '|'
            << p->trans_alt << '|' << p->trans_level << '|' << p->length << '|';
        if(p->getRSBN())
        {
            out << p->getRSBN()->wpOrigin << '|' << p->getRSBN()->name << '|' << p->getRSBN()->name2 << '|' << p->getRSBN()->freq << '|' << p->getRSBN()->latlon.x << '|' << p->getRSBN()->latlon.y << "\n";
        }//if
        else
        {
            out << 0 << '|' << "" << '|' << "" << '|' << 0 << '|' << 0 << '|' << 0 << "\n";
        }
    }

    outfile.close();

    return 1;
}

std::vector<NVUPOINT*> XFMS_DATA::loadXNVUFlightplan(const QString& file)
{
    std::vector<NVUPOINT*> lWPS;
    QFile infile(file);
    if(!infile.open(QIODevice::ReadOnly | QIODevice::Text)) return lWPS;

    NVUPOINT* p;
    while(!infile.atEnd())
    {
        QString line = infile.readLine();
        QStringList list;
        list = line.split('|');//QString::SkipEmptyParts);
        validate_xnvuflightplan(lWPS, list);
    }//while

    infile.close();

    return lWPS;
}

void XFMS_DATA::validate_xnvuflightplan(std::vector<NVUPOINT*>& lXNVUFlightplan, const QStringList& RAW)
{
    //std::printf("XMFS_DATA::validate_xnvuflightplan %d\n", RAW.size());
    if(RAW.size()<19) return;

    NVUPOINT wp;
    NVUPOINT rsbn;

    for(int i=0; i<RAW.size(); i++)
    {
        QString qstr = RAW.at(i).simplified();
        switch(i)
        {
            case 0: //TYPE
                wp.type = qstr.toInt();
            break;
            case 1: //Identifier
                wp.name = qstr;
            break;
            case 2: //Name
                wp.name2 = qstr;
            break;
            case 3: //Country
                wp.country = qstr;
            break;
            case 4: //Lat
                wp.latlon.x = qstr.toDouble();
            break;
            case 5: //Long
                wp.latlon.y = qstr.toDouble();
            break;
            case 6: //Altitude
                wp.alt = qstr.toInt();
            break;
            case 7: //Elev
                wp.elev = qstr.toInt();
            break;
            case 8: //Freq
                wp.freq = qstr.toDouble();
            break;
            case 9: //Range
                wp.range = qstr.toInt();
            break;
            case 10: //Angle Deviation
                wp.ADEV = qstr.toDouble();
            break;
            case 11: //Transition Alt
                wp.trans_alt = qstr.toInt();
            break;
            case 12: //Tansition Level
                wp.trans_level = qstr.toInt();
            break;
            case 13: //Longest Runway
                wp.length = qstr.toInt();
            break;
            case 14: //RSBN origin
                rsbn.wpOrigin = qstr.toInt();
            break;
            case 15: //Identifier
                rsbn.name = qstr;
            break;
            case 16: //Name
                rsbn.name2 = qstr;
            break;
            case 17: //Freq
                rsbn.freq = qstr.toDouble();
            break;
            case 18: //Lat
                rsbn.latlon.x = qstr.toDouble();
            break;
            case 19: //Long
                rsbn.latlon.y = qstr.toDouble();
            break;
        }//Switch
    }//for

    //Search for RSBN correction beacon, it needs to be the one here.
    std::vector< std::pair<NVUPOINT*, double> > lR;
    lR = XFMS_DATA::getClosestRSBN(&rsbn, -1, -1, (rsbn.wpOrigin == WAYPOINT::ORIGIN_RSBN) ? false : true);
    if(lR.size()>0)
    {
        if
        (
           rsbn.name.compare(lR[0].first->name)==0 &&
           rsbn.name2.compare(lR[0].first->name2)==0 &&
           rsbn.freq == lR[0].first->freq &&
           lR[0].second<=DEFAULT_WAYPOINT_MARGIN
        )
        {
            wp.setRSBN(lR[0].first);
        }
    }

    wp.wpOrigin = WAYPOINT::ORIGIN_XWP;
    wp.MD = calc_magvar(wp.latlon.x, wp.latlon.y, dat, (double(LMATH::feetToMeter(wp.alt))/1000.0));

    NVUPOINT* nwp = NULL;
    if(DialogSettings::distAlignXWP)
    {
        double d = -1;
        std::vector< std::pair<NVUPOINT*, double> > lWP = getClosestWaypointType(wp.latlon, wp.name, 0);
        if(lWP.size()>0){ nwp = lWP[0].first; d = lWP[0].second; };

        if(d>DialogSettings::distAlignMargin || nwp==NULL) nwp = new NVUPOINT(wp);
        else
        {
            nwp = new NVUPOINT(*nwp);
            nwp->setRSBN(wp.getRSBN());
        }
    }//if
    else nwp = new NVUPOINT(wp);

    lXNVUFlightplan.push_back(nwp);
}


std::vector<NVUPOINT*> XFMS_DATA::loadFMS(const QString& file)
{
    std::vector<NVUPOINT*> lFMS;

    QFile infile(file);
    if(!infile.open(QIODevice::ReadOnly | QIODevice::Text)) return lFMS;

    while(!infile.atEnd())
    {
        QString line = infile.readLine();
        QStringList list;
        list = line.split(' ', QString::SkipEmptyParts);

        if(list.size() == 5)
        {
            validate_fms(lFMS, list);
        }

    }//while

    infile.close();
    return lFMS;
}//void load

int XFMS_DATA::saveFMS(const QString& file, const std::vector<NVUPOINT*> lN)
{
    //Type | ID | ALT | LAT | LON
    QFile outfile(file);
    if(!outfile.open(QIODevice::WriteOnly | QIODevice::Text)) return 0;


    QTextStream out(&outfile);

    out << "I\n3 version\n1\n" << lN.size() << '\n';
    for(int i=0; i<lN.size(); i++)
    {
        NVUPOINT* p = lN[i];
        out <<  qSetRealNumberPrecision(16) << XNVUToFMSType(p->type) << ' ' << p->name << ' ' << p->alt << ' ' << p->latlon.x << ' ' << p->latlon.y << '\n';
    }

    outfile.close();

    return 1;
}

void XFMS_DATA::validate_fms(std::vector<NVUPOINT*>& lFMS, const QStringList& RAW)
{
    NVUPOINT wp;

    for(int i=0; i<RAW.size(); i++)
    {
        QString qstr = RAW.at(i).simplified();
        switch(i)
        {
            case 0: //TYPE
                wp.type = XFMS_DATA::FMSToXNVUType(qstr.toInt());
            break;
            case 1:
                wp.name = qstr;
            break;
            case 2: //Altitude (feet)
                wp.alt = qstr.toDouble();
            break;

            case 3:
                wp.latlon.x = qstr.toDouble();
            break;
            case 4:
                wp.latlon.y = qstr.toDouble();
            break;
        }//Switch
    }//for
    wp.wpOrigin = WAYPOINT::ORIGIN_FMS;
    wp.MD = calc_magvar(wp.latlon.x, wp.latlon.y, dat, (double(LMATH::feetToMeter(wp.alt))/1000.0));

    NVUPOINT* nwp;
    if(DialogSettings::distAlignFMS)
    {
        double d = 0.0;
        std::vector< std::pair<NVUPOINT*, double> > lWP = getClosestWaypointType(wp.latlon, wp.name, 0);
        if(lWP.size()>0){ nwp = lWP[0].first; d = lWP[0].second; };

        if(d>DialogSettings::distAlignMargin || nwp==NULL) nwp = new NVUPOINT(wp);
        else
        {
            nwp = new NVUPOINT(*nwp);
        }
    }//if
    else nwp = new NVUPOINT(wp);


    //lWP.insert(std::make_pair(nwp->name, nwp));
    //lWP2.insert(std::make_pair(nwp->name2, nwp));
    lFMS.push_back(nwp);
}

int XFMS_DATA::FMSToXNVUType(int _type)
{
    switch(_type)
    {
        case 1: return WAYPOINT::TYPE_AIRPORT;
        case 2: return WAYPOINT::TYPE_NDB;
        case 3: return WAYPOINT::TYPE_VOR;
        case 11: return WAYPOINT::TYPE_FIX;
        case 28: return WAYPOINT::TYPE_LATLON;
    };

    return _type;
}

int XFMS_DATA::XNVUToFMSType(int _type)
{
    if(_type == WAYPOINT::TYPE_AIRPORT) return 1;
    if(_type == WAYPOINT::TYPE_NDB) return 2;
    if(_type == WAYPOINT::TYPE_RSBN) return 3;
    if(_type == WAYPOINT::TYPE_VORDME) return 3;
    if(_type == WAYPOINT::TYPE_VOR) return 3;
    if(_type == WAYPOINT::TYPE_ILS) return 3;
    if(_type == WAYPOINT::TYPE_FIX) return 11;
    if(_type == WAYPOINT::TYPE_DME) return 28;
    if(_type == WAYPOINT::TYPE_LATLON) return 28;
    if(_type == WAYPOINT::TYPE_TACAN) return 28;
    if(_type == WAYPOINT::TYPE_VORTAC) return 3;
    if(_type == WAYPOINT::TYPE_VHFNAV) return 28;

    return 0;
}


