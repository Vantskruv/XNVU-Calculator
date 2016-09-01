#include "NVU.h"
#include <iostream>
#include <cmath>
#include <QString>
#include "LMATH.h"
#include "coremag.h"
#include <QDebug>

void NVUPOINT::calc_rsbn_corr(CPoint p2R)
{
    if(rsbn==NULL) return;

    //Set coordinates to radial
    CPoint pRR = rsbn->latlon*M_PI/180.0;
    CPoint p1R = latlon*M_PI/180.0;
    p2R = p2R*M_PI/180.0;

    //Calculate point on course which is in 90 degree angle to the RSBN (vI)
    CPoint v = LMATH::latlonToVector(p2R) - LMATH::latlonToVector(p1R);       //Vector v pointing from p1R to p2R
    CPoint vR = LMATH::latlonToVector(pRR) - LMATH::latlonToVector(p1R);                        //Vector vR pointing from p1R to pRR
    double d = LMATH::GetClosestPointOnVector(vR, v);                                           //Returns scalar of v where vR is closest
    CPoint vI = LMATH::latlonToVector(p1R) + v*d;                                               //Scale vector v to closest point and add p1R
    vI = LMATH::vectorToLatlon(vI);                                                             //Transform above to latlon

    //Calculate Sm (distance from second point to vI)
    Sm = LMATH::calc_distance(p2R*180.0/M_PI, vI*180.0/M_PI);
    //If closest point on course is at the other side of endpoint, set Sm to negative.
    if(d<1.0) Sm = -Sm;

    //Calculate Zm (distance from RSBN to vI)
    Zm = LMATH::calc_distance(rsbn->latlon, vI*180.0/M_PI);
    //If RSBN is on the left side of course, set Zm to negative.
    v = v.getNormal(true);
    if(LMATH::GetClosestPointOnVector(vR, v)>0) Zm = -Zm;

    //Calculate map angle
    //Testing
    /*
    vI = LMATH::latlonToVector(vI) - LMATH::latlonToVector(pRR);
    CPoint vII = pRR;
    vII .x = 90.0;
    vII = LMATH::latlonToVector(vII);
    MapAngle = vII.getAngle(vI)*180/M_PI;
    if(MapAngle<0) MapAngle+=360;
    */

    vI = LMATH::latlonToVector(p2R) - LMATH::latlonToVector(p1R) + LMATH::latlonToVector(pRR);
    vI = LMATH::vectorToLatlon(vI);
    vI = vI*180.0/M_PI;
    MapAngle = LMATH::calc_bearing(rsbn->latlon, vI);       //YK - A????
    if(rsbn->wpOrigin == WAYPOINT::ORIGIN_EARTHNAV) MapAngle-=rsbn->ADEV;

    //Calculate Atrg
    Atrg = LMATH::calc_bearing(rsbn->latlon, latlon);

    //Calculate Dtrg
    Dtrg = LMATH::calc_distance(rsbn->latlon, latlon);

}

void NVU::generate(const std::vector< std::pair<NVUPOINT*, NVUPOINT*> > _lWPs, long dat)
{
	if(_lWPs.size()<2) return;

	for(int i=0; i<_lWPs.size(); i++)
	{
        NVUPOINT* nvup = new NVUPOINT(*_lWPs[i].first);
        nvup->rsbn = _lWPs[i].second;
		lWPs.push_back(nvup);
	}//for


    NVUPOINT& wp1= *lWPs[0];
    NVUPOINT& wp2 = *lWPs[lWPs.size()-1];
    NVU_FORK = LMATH::calc_fork(wp1.latlon.x, wp1.latlon.y, wp2.latlon.x, wp2.latlon.y, dat);

	double Spas = 0;
	double e = 0;
	int i=0;		
    for(; i<lWPs.size()-1; i++)
	{
        CPoint a = lWPs[i]->latlon;
        CPoint b = lWPs[i+1]->latlon;


        if(a.isSimilar(b, 0.0001))
        {
            lWPs[i]->MD = 0;
            lWPs[i]->IPU = 0;
            lWPs[i]->MPU = 0;
            lWPs[i]->OZMPUv = 0;
            lWPs[i]->OZMPUp = 0;
            lWPs[i]->Pv = 0;
            lWPs[i]->Pp = 0;
            lWPs[i]->Spas = 0;
            lWPs[i]->S = 0;
            lWPs[i]->LPT = 0;
            //qDebug() << i << "  is similar to  " << i+1 << " Dist = " << LMATH::calc_distance(a, b)<<"\n";
            continue;
        }


        lWPs[i]->MD = calc_magvar(a.x, a.y, dat);
        lWPs[i]->IPU = LMATH::calc_bearing(a, b);
        lWPs[i]->MPU = LMATH::angleTo360(lWPs[i]->IPU - lWPs[i]->MD);
        lWPs[i]->OZMPUv = LMATH::angleTo360(lWPs[i]->IPU - lWPs[0]->MD);

        //qDebug() << "I: " << i << ", " << "E: " << e << ", B: " << LMATH::calc_bearing(b, a)<<'\n';
        lWPs[i]->OZMPUv = LMATH::angleTo360(lWPs[i]->OZMPUv + e);

        e = LMATH::calc_bearing(b, a) - lWPs[0]->MD - 180;
        e = LMATH::angleTo360(e);
        e = lWPs[i]->OZMPUv - e;

        lWPs[i]->OZMPUp = LMATH::angleTo360(lWPs[i]->OZMPUv + NVU_FORK);

        lWPs[i]->Pv = lWPs[i]->MPU - lWPs[i]->OZMPUv;
        lWPs[i]->Pp = lWPs[i]->MPU - lWPs[i]->OZMPUp;
        lWPs[i]->Spas = Spas;
        lWPs[i]->S = LMATH::calc_distance(a.x, a.y, b.x, b.y);
        Spas+= lWPs[i]->S;
        lWPs[i]->LPT = 5;	//TODO Probably turndistance, and I guess maybe S will be changed depending on this value.


        /*
        if(lWPs[i]->IPU<0) lWPs[i]->IPU+=360;
        if(lWPs[i]->MPU<0) lWPs[i]->MPU+=360;
        if(lWPs[i]->OZMPUv<0) lWPs[i]->OZMPUv+=360;
        if(lWPs[i]->OZMPUp<0) lWPs[i]->OZMPUp+=360;
        */

	}//for
	
    lWPs[i]->S = 0;
    lWPs[i]->Spas = Spas;
    lWPs[i]->MD = calc_magvar(lWPs[i]->latlon.x, lWPs[i]->latlon.y, dat);

	for(i=0 ; i<lWPs.size()-1; i++)
	{
        lWPs[i]->Srem = Spas;
        Spas-=lWPs[i]->S;
	}//


    for(int i=0; i<lWPs.size() - 1; i++)
    {
        if(lWPs[i]->latlon.isSimilar(lWPs[i+1]->latlon, 0.0001)) continue;
        lWPs[i]->calc_rsbn_corr(lWPs[i+1]->latlon);
    }
	
}//void

/*
    if(_lWPs.size()<2) return;

    for(int i=0; i<_lWPs.size(); i++)
    {
        NVUPOINT* nvup = new NVUPOINT(*_lWPs[i].first);
        nvup->rsbn = _lWPs[i].second;
        lWPs.push_back(nvup);
    }//for


    NVUPOINT& wp1= *lWPs[0];
    NVUPOINT& wp2 = *lWPs[lWPs.size()-1];
    NVU_FORK = LMATH::calc_fork(wp1.latlon.x, wp1.latlon.y, wp2.latlon.x, wp2.latlon.y, dat);

    double Spas = 0;
    double e = 0;
    int i=0;
    for(; i<lWPs.size()-1; i++)
    {
        CPoint a = lWPs[i]->latlon;
        CPoint b = lWPs[i+1]->latlon;


        lWPs[i]->MD = calc_magvar(a.x, a.y, dat);


        lWPs[i]->IPU = LMATH::calc_bearing(a, b);
        lWPs[i]->MPU = lWPs[i]->IPU - lWPs[i]->MD;
        if(lWPs[i]->MPU<0) lWPs[i]->MPU+=360;

        lWPs[i]->OZMPUv = LMATH::calc_bearing(a, b) - lWPs[0]->MD;
        if(lWPs[i]->OZMPUv<0) lWPs[i]->OZMPUv+=360;

        //qDebug() << "E: " << e << '\n';
        if(i>0) lWPs[i]->OZMPUv+=e;


        lWPs[i]->OZMPUp = lWPs[i]->OZMPUv + NVU_FORK;
        if(lWPs[i]->OZMPUp<0) lWPs[i]->OZMPUp+=360;

        e = LMATH::calc_bearing(b, a) - lWPs[0]->MD;
        e = lWPs[i]->OZMPUv - e - 180;
        if(e<0) e+=360;


        lWPs[i]->Pv = lWPs[i]->MPU - lWPs[i]->OZMPUv;
        lWPs[i]->Pp = lWPs[i]->MPU - lWPs[i]->OZMPUp;
        lWPs[i]->Spas = Spas;
        lWPs[i]->S = LMATH::calc_distance(a.x, a.y, b.x, b.y);
        Spas+= lWPs[i]->S;
        lWPs[i]->LPT = 5;	//TODO Probably turndistance, and I guess maybe S will be changed depending on this value.


        //if(lWPs[i]->IPU<0) lWPs[i]->IPU+=360;
        //if(lWPs[i]->MPU<0) lWPs[i]->MPU+=360;
        //if(lWPs[i]->OZMPUv<0) lWPs[i]->OZMPUv+=360;
        //if(lWPs[i]->OZMPUp<0) lWPs[i]->OZMPUp+=360;


    }//for

    lWPs[i]->S = 0;
    lWPs[i]->Spas = Spas;
    //lWPs[i]->MD = calc_magvar(lWPs[i]->latlon.x, lWPs[i]->latlon.x, dat);

    for(i=0 ; i<lWPs.size()-1; i++)
    {
        lWPs[i]->Srem = Spas;
        Spas-=lWPs[i]->S;
    }//


    for(int i=0; i<lWPs.size() - 1; i++)
    {
        lWPs[i]->calc_rsbn_corr(lWPs[i+1]->latlon);
    }

}//void

*/


void NVU::calc_rsbn_corr(NVUPOINT* nP, NVUPOINT* rsbn) //Set RSBN for NVUPOINT and calculate correction
{
    nP->rsbn = rsbn;
    for(int i=0; i<lWPs.size(); i++)
    {
        if(lWPs[i] == nP)
        {
            nP->calc_rsbn_corr(lWPs[i]->latlon);
            break;
        }
    }
}

void NVU::clear()
{
    NVU_FORK = 0;
    for(int i=0; i<lWPs.size(); i++) delete lWPs[i];

    lWPs.clear();
}

/*
void NVU::printWaypoints()
{

	//std::cout.fixed;
	std::cout.precision(4);
	std::cout<< "FORK: " << NVU_FORK << std::endl;
	
	for(int i=0; i<lWPs.size(); i++)
	{
		NVUPOINT wp = lWPs[i];

		std::printf("%s MD: %.1f Uv: %.1f Up: %.1f MPU: %.1f IPU: %.1f Pv: %.1f Pp: %.1f \n", wp.name.c_str(), wp.MD, wp.OZMPUv, wp.OZMPUp, wp.MPU, wp.IPU, wp.Pv, wp.Pp);

		//std::cout << wp.name << " MD:" << wp.MD << " Uv:" << wp.OZMPUv << " Up:" << wp.OZMPUp << " MPU: " << wp.MPU << " IPU: " << wp.IPU << " Pv: " << wp.Pv << " Pp: " << wp.Pp << std::endl;
		// " S:"<< wp.S << " Spas:" << wp.Spas << " Srem:" << wp.Srem << std::endl;
	}
}
*/
