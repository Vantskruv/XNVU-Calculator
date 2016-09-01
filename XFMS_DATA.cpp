#include "XFMS_DATA.h"

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <iostream>

#include <list>
#include <QString>
#include <QFile>
#include <QTextStream>
#include "LMATH.h"
#include "coremag.h"
#include <algorithm>
#include "dialogsettings.h"
#include <QDebug>

std::multimap<QString, NVUPOINT*> XFMS_DATA::lWP;
std::multimap<QString, NVUPOINT*> XFMS_DATA::lWP2;
int XFMS_DATA::dat;
//Loaded from i.e. navigraph
std::vector<NVUPOINT*> XFMS_DATA::lAirports;
std::vector<NVUPOINT*> XFMS_DATA::lNDB;
std::vector<NVUPOINT*> XFMS_DATA::lVOR;
std::vector<NVUPOINT*> XFMS_DATA::lDME;
std::vector<NVUPOINT*> XFMS_DATA::lVORDME;         //Cannot be used as RSBN, as there is no angle deviation data.
std::vector<NVUPOINT*> XFMS_DATA::lFixes;

//Loaded from earthnav.dat
std::vector<NVUPOINT*> XFMS_DATA::lXNDB;
std::vector<NVUPOINT*> XFMS_DATA::lXVOR;
std::vector<NVUPOINT*> XFMS_DATA::lXDME;
std::vector<NVUPOINT*> XFMS_DATA::lXVORDME;        //VOR/DME loaded from X-Plane earth earth_nav.dat, as it includes VOR angle deviation data.

//Loaded from rsbn.dat
std::vector<NVUPOINT*> XFMS_DATA::lRSBN;

//Loaded from xnvu.dat
std::vector<NVUPOINT*> XFMS_DATA::lXNVU;

//Loaded from user fms file
std::vector<NVUPOINT*> XFMS_DATA::lFMS;

//Loaded from user xnvu.wps file
std::vector<NVUPOINT*> XFMS_DATA::lXNVUFlightplan;

XFMS_DATA::XFMS_DATA()
{

}

XFMS_DATA::~XFMS_DATA()
{
    clear();
}

void XFMS_DATA::clear()
{
    lAirports.clear();
    lNDB.clear();
    lVOR.clear();
    lDME.clear();
    lVORDME.clear();
    lFixes.clear();

    lXNDB.clear();
    lXVOR.clear();
    lXDME.clear();
    lXVORDME.clear();

    lRSBN.clear();

    lXNVU.clear();
    lFMS.clear();
    lXNVUFlightplan.clear();

    lWP2.clear();
    std::multimap<QString, NVUPOINT*>::iterator it = lWP.begin();
    for(; it!=lWP.end(); it++)
    {
        delete it->second;
    }
    lWP.clear();
}

std::vector<NVUPOINT*> XFMS_DATA::search(const QString& _name)
{
    std::vector<NVUPOINT*> rWP;

    if(_name.isEmpty()) return rWP;

    std::pair<std::multimap<QString, NVUPOINT*>::iterator, std::multimap<QString, NVUPOINT*>::iterator> ret = lWP.equal_range(_name);
    for(std::multimap<QString, NVUPOINT*>::iterator it=ret.first; it!=ret.second; it++)
	{
        NVUPOINT* wp = it->second;
        rWP.push_back(wp);
	}

    ret = lWP2.equal_range(_name);
    for(std::multimap<QString, NVUPOINT*>::iterator it=ret.first; it!=ret.second; it++)
    {
        NVUPOINT* wp = it->second;
        rWP.push_back(wp);
    }

    //std::printf("rWP.size: %d\n", rWP.size());

	return rWP;
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


int XFMS_DATA::_load(const QString& file, int type)
{
    QFile infile(file);
    if(!infile.open(QIODevice::ReadOnly | QIODevice::Text)) return 0;

    int li = 0;

    while(!infile.atEnd())
	{
        QString line = infile.readLine();
        QStringList list;
        if(type == 3 || type == 5) list = line.split('|');//, QString::SkipEmptyParts);           //RSBN data (rsbn.dat)
        else if(type == 4) list = line.split(' ', QString::SkipEmptyParts);      //X-Plane navdata (earth_nav.dat)
        else list = line.split(',',  QString::SkipEmptyParts);

        switch(type)
		{
            case 0: validate_airport(list); break;
            case 1: validate_navaid(list); break;
            case 2: validate_waypoint(list); break;
            case 3: validate_RSBN(list); break;
            case 4: validate_earthnav(list); break;
            case 5: validate_xnvu(list); break;
		}//switch

        li++;
	}//while

	infile.close();

    return 1;
}//void load

QString XFMS_DATA::load(int _dat)
{
    dat = _dat;

/*
    if(!_load(directory + "Airports.txt", 0))   re = 0x00001;
    if(!_load(directory + "Navaids.txt", 1))    re&= 0x00010;
    if(!_load(directory + "Waypoints.txt", 2))  re&= 0x00100;
    if(!_load(directory + "rsbn.dat", 3))       re&= 0x01000;
    if(!_load(directory + "earth_nav.dat", 4))  re&= 0x10000;
*/

    QString sError;
    if(!_load(DialogSettings::fileAirports, 0))
    {
        sError = " [Airports.txt]";
    }
    if(!_load(DialogSettings::fileNavaids, 1))
    {
        sError = sError + " [Navaids.txt]";
    }
    if(!_load(DialogSettings::fileWaypoints, 2))
    {
        sError = sError + " [Waypoints.txt]";
    }
    if(!_load(DialogSettings::fileRSBN, 3))
    {
        sError = sError + " [rsbn.dat]";
    }
    if(!_load(DialogSettings::fileNavdata, 4))
    {
        sError = sError + " [earth_nav.txt]";
    }
    if(!_load(XNVU_WPS_FILENAME, 5))
    {
        sError = sError + " [xnvu_wps.txt]";

    }

    return sError;
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


void XFMS_DATA::validate_airport(const QStringList& record)
{
    if(record.size()<5) return;

    NVUPOINT* wp = new NVUPOINT();
    wp->type = WAYPOINT::TYPE_AIRPORT;

	for(int i=0; i<record.size(); i++)
	{
        QString qstr = record.at(i).simplified();
        switch(i)
		{
			case 0:	//Runway or airport
                if(qstr.compare("A")!=0) return;
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
                wp->longest_runway = qstr.toInt();
			break;
		}//switch
	}//for

    wp->wpOrigin = WAYPOINT::ORIGIN_AIRAC_AIRPORTS;
    wp->MD = calc_magvar(wp->latlon.x, wp->latlon.y, dat);
    lWP.insert(std::make_pair(wp->name, wp));
    lWP2.insert(std::make_pair(wp->name2, wp));
    lAirports.push_back(wp);
}

void XFMS_DATA::validate_navaid(const QStringList &record)
{
	if(record.size()<9) return;

    NVUPOINT* wp = new NVUPOINT();

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
			break;
			case 2: //Frequency
                wp->freq =qstr.toDouble();
			break;
            case 3: //Is VOR?
                wp->type = (qstr.toInt()) ? WAYPOINT::TYPE_VOR : 0;
			break;
            case 4:	//Is DME?
                if(qstr.toInt())
                {
                    wp->type = (wp->type) ? WAYPOINT::TYPE_VORDME : WAYPOINT::TYPE_DME;  //Is VORDME or DME
                }
                else if(!wp->type) wp->type = WAYPOINT::TYPE_NDB;   //Is NDB
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

    wp->wpOrigin = WAYPOINT::ORIGIN_AIRAC_NAVAIDS;
    wp->MD = calc_magvar(wp->latlon.x, wp->latlon.y, dat);
    lWP.insert(std::make_pair(wp->name, wp));
    lWP2.insert(std::make_pair(wp->name2, wp));

    if(wp->type == WAYPOINT::TYPE_NDB)    lNDB.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_VOR)    lVOR.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_DME)    lDME.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_VORDME) lVORDME.push_back(wp);
}

void XFMS_DATA::validate_earthnav(const QStringList &record)
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
            case 6: //Range
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

    /*
    //Search and add real name and country
    double dMargin = 0.1;
    double dMin = std::numeric_limits<double>::max();
    NVUPOINT* cWP = NULL;
    std::vector<NVUPOINT*> lS = search(wp->name);
    for(int i=0; i<lS.size(); i++)
    {
        double d = LMATH::calc_distance(wp->latlon, lS[i]->latlon);
        if(d<dMin)
        {
            dMin = d;
            cWP = lS[i];
        }
    }

    if(cWP)
    {
        if(dMin<dMargin)
        {
            wp->country = cWP->country;
        }
    }
    */


    wp->wpOrigin = WAYPOINT::ORIGIN_EARTHNAV;
    wp->MD = calc_magvar(wp->latlon.x, wp->latlon.y, dat);
    lWP.insert(std::make_pair(wp->name, wp));
    lWP2.insert(std::make_pair(wp->name2, wp));

    if(wp->type == WAYPOINT::TYPE_NDB) lXNDB.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_VORDME) lXVORDME.push_back(wp);
    else if(wp->type == WAYPOINT::TYPE_DME) lXDME.push_back(wp);
}

void XFMS_DATA::validate_waypoint(const QStringList& record)
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

    wp->wpOrigin = WAYPOINT::ORIGIN_AIRAC_WAYPOINTS;
    wp->MD = calc_magvar(wp->latlon.x, wp->latlon.y, dat);
    lWP.insert(std::make_pair(wp->name, wp));
    lWP2.insert(std::make_pair(wp->name2, wp));
    lFixes.push_back(wp);
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
                wp.longest_runway = qstr.toInt();
            break;
        }//Switch
    }//for
    wp.wpOrigin = WAYPOINT::ORIGIN_XNVU;
    wp.MD = calc_magvar(wp.latlon.x, wp.latlon.y, dat);

    NVUPOINT* nwp = new NVUPOINT(wp);

    lWP.insert(std::make_pair(nwp->name, nwp));
    lWP2.insert(std::make_pair(nwp->name2, nwp));
    lXNVU.push_back(nwp);
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
            << p->trans_alt << '|' << p->trans_level << '|' << p->longest_runway << "\n";
    }

    outfile.close();
    return 1;
}

int XFMS_DATA::saveXNVUFlightplan(const QString& file, std::vector<NVUPOINT*> lN)
{

    //Type | ID | NAME | COUNTRY | LAT | LON | ELEV | FREQ | RANGE | AD | TALT | TLVL | LRWY | RSBN_ORIGIN | RSBN_ID | RSBN_NAME | RSBN_FREQ | RSBN_LAT | RSBN_LON
    QFile outfile(file);
    if(!outfile.open(QIODevice::WriteOnly | QIODevice::Text)) return 0;


    QTextStream out(&outfile);

    for(int i=0; i<lN.size(); i++)
    {
        NVUPOINT* p = lN[i];
        out << qSetRealNumberPrecision(16)
            << p->type << '|' << p->name << '|' << p->name2 << '|' << p->country << '|' << p->latlon.x << '|' << p->latlon.y << '|'
            << p->elev << '|' << p->freq << '|' << p->range << '|' << p->ADEV << '|'
            << p->trans_alt << '|' << p->trans_level << '|' << p->longest_runway << '|';
        if(p->rsbn)
        {
            out << p->rsbn->wpOrigin << '|' << p->rsbn->name << '|' << p->rsbn->name2 << '|' << p->rsbn->freq << '|' << p->rsbn->latlon.x << '|' << p->rsbn->latlon.y << "\n";
        }//if
        else
        {
            out << 0 << '|' << "" << '|' << "" << '|' << 0 << '|' << 0 << '|' << 0 << "\n";
        }
    }

    outfile.close();

    return 1;
}

int XFMS_DATA::loadXNVUFlightplan(const QString& file)
{
    std::vector<NVUPOINT*> lN;
    QFile infile(file);
    if(!infile.open(QIODevice::ReadOnly | QIODevice::Text)) return 0;

    NVUPOINT* p;
    while(!infile.atEnd())
    {
        QString line = infile.readLine();
        QStringList list;
        list = line.split('|');//QString::SkipEmptyParts);
        validate_xnvuflightplan(list);
    }//while

    infile.close();

    return 1;
}

void XFMS_DATA::removeXNVUFlightplan()
{
    /*
    std::pair<std::multimap<QString, NVUPOINT*>::iterator, std::multimap<QString, NVUPOINT*>::iterator> ret = lWP.equal_range(_name);
    std::multimap<QString, NVUPOINT*>::iterator iWp;
    for(int i=0; i<lXNVUFlightplan.size(); i++)
    {
        ret = lWP.equal_range(lXNVUFlightplan[i]->name);
        for(iWp=ret.first; iWp!=ret.second; iWp++)
        {
            if(iWp.second == lXNVUFlightplan[i]) lWP.erase(iWp);
        }

        ret = lWP2.equal_range(lXNVUFlightplan[i]->name2);
        for(iWp=ret.first; iWp!=ret.second; iWp++)
        {
            if(iWp.second == lXNVUFlightplan[i]) lWP.erase(iWp);
        }
    }
    */

    for(int i=0; i<lXNVUFlightplan.size(); i++) delete lXNVUFlightplan[i];
    lXNVUFlightplan.clear();
}

void XFMS_DATA::validate_xnvuflightplan(const QStringList& RAW)
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
                wp.longest_runway = qstr.toInt();
            break;
            case 13: //RSBN origin
                rsbn.wpOrigin = qstr.toInt();
            break;
            case 14: //Identifier
                rsbn.name = qstr;
            break;
            case 15: //Name
                rsbn.name2 = qstr;
            break;
            case 16: //Freq
                rsbn.freq = qstr.toDouble();
            break;
            case 17: //Lat
                rsbn.latlon.x = qstr.toDouble();
            break;
            case 18: //Long
                rsbn.latlon.y = qstr.toDouble();
            break;
        }//Switch
    }//for

    //Search for RSBN correction beacon, it needs to be the one here.
    std::vector< std::pair<NVUPOINT*, double> > lR;
    lR = XFMS_DATA::getClosestRSBN(&rsbn, -1, -1, (rsbn.wpOrigin == WAYPOINT::ORIGIN_RSBN) ? false : true);
    wp.rsbn = NULL;
    if(lR.size()>0)
    {
        if
        (
           rsbn.name.compare(lR[0].first->name)==0 &&
           rsbn.name2.compare(lR[0].first->name2)==0 &&
           rsbn.freq == lR[0].first->freq &&
           lR[0].second<0.1
        )
        {
            wp.rsbn = lR[0].first;
        }
    }

    wp.wpOrigin = WAYPOINT::ORIGIN_XNVU_FLIGHTPLAN;
    wp.MD = calc_magvar(wp.latlon.x, wp.latlon.y, dat);

    NVUPOINT* nwp = new NVUPOINT(wp);
    //lWP.insert(std::make_pair(nwp->name, nwp));
    //lWP2.insert(std::make_pair(nwp->name2, nwp));
    lXNVUFlightplan.push_back(nwp);
}


int XFMS_DATA::loadFMS(const QString& file)
{
    QFile infile(file);
    if(!infile.open(QIODevice::ReadOnly | QIODevice::Text)) return 0;

    while(!infile.atEnd())
    {
        QString line = infile.readLine();
        QStringList list;
        list = line.split(' ', QString::SkipEmptyParts);

        if(list.size() == 5)
        {
            validate_fms(list);
        }

    }//while

    infile.close();
    return 1;
}//void load

int XFMS_DATA::saveFMS(const QString& file, std::vector<NVUPOINT*> lN)
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

void XFMS_DATA::validate_fms(const QStringList& RAW)
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
    wp.MD = calc_magvar(wp.latlon.x, wp.latlon.y, dat);

    NVUPOINT* nwp = new NVUPOINT(wp);
    //lWP.insert(std::make_pair(nwp->name, nwp));
    //lWP2.insert(std::make_pair(nwp->name2, nwp));
    lFMS.push_back(nwp);
}

void XFMS_DATA::removeFMS()
{
    for(int i=0; i<lFMS.size(); i++) delete lFMS[i];
    lFMS.clear();
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
    if(_type == WAYPOINT::TYPE_FIX) return 11;
    if(_type == WAYPOINT::TYPE_DME) return 28;
    if(_type == WAYPOINT::TYPE_LATLON) return 28;

    return 0;
}


