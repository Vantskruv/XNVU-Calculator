#include "NVU.h"
#include <iostream>
#include <cmath>
#include <QString>
#include "LMATH.h"
#include "coremag.h"
#include <QDebug>



void NVU::generate(std::vector<NVUPOINT*>& lWPs, double& NVU_FORK, long dat)
{
    if(lWPs.size()<2) return;

    /*
	for(int i=0; i<_lWPs.size(); i++)
	{
        NVUPOINT* nvup = new NVUPOINT(*_lWPs[i].first);
        nvup->MD = calc_magvar(nvup->latlon.x, nvup->latlon.y, dat, (double(LMATH::feetToMeter(nvup->alt))/1000.0));
        nvup->rsbn = _lWPs[i].second;
		lWPs.push_back(nvup);
	}//for
    */

    //Calculate magnetic declination at the given position and altitude for every point
    for(int i=0; i<lWPs.size(); i++)
    {
        NVUPOINT* nvup = lWPs[i];
        nvup->MD = calc_magvar(nvup->latlon.x, nvup->latlon.y, dat, (double(LMATH::feetToMeter(nvup->alt))/1000.0));
    }//for

    NVUPOINT& wp1= *lWPs[0];
    NVUPOINT& wp2 = *lWPs[lWPs.size()-1];
    NVU_FORK = LMATH::calc_fork(wp1.latlon.x, wp1.latlon.y, wp1.alt, wp2.latlon.x, wp2.latlon.y, wp2.alt, dat);

	double Spas = 0;
	double e = 0;
	int i=0;		
    for(; i<int(lWPs.size())-1; i++)
	{
        CPoint a = lWPs[i]->latlon;
        CPoint b = lWPs[i+1]->latlon;


        //It may get nasty if waypoints are unbelievable close.
        if(a.isSimilar(b, DEFAULT_WAYPOINT_MARGIN))
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
            continue;
        }


        lWPs[i]->IPU = LMATH::calc_bearing(a, b);                               //True course
        lWPs[i]->MPU = LMATH::angleTo360(lWPs[i]->IPU - lWPs[i]->MD);           //Magnetic course

        lWPs[i]->OZMPUv = LMATH::angleTo360(lWPs[i]->IPU - lWPs[0]->MD);
        lWPs[i]->OZMPUv = LMATH::angleTo360(lWPs[i]->OZMPUv + e);               //Orthodromic course

        //Azimuth
        e = LMATH::calc_bearing(wp1.latlon, b) - (LMATH::calc_bearing(b, wp1.latlon) - 180);
        /*
        Original
        e = LMATH::calc_bearing(b, a) - lWPs[0]->MD - 180;
        e = LMATH::angleTo360(e);
        e = lWPs[i]->OZMPUv - e;
        qDebug() << "Eo[" << i << "] = " << e;
        */


        lWPs[i]->OZMPUp = LMATH::angleTo360(lWPs[i]->OZMPUv + NVU_FORK);        //Orthodromic course with added fork of departure and arrival

        //Azimuth difference
        //lWPs[i]->Pv = lWPs[i]->MPU - lWPs[i]->OZMPUv; //Deprecated
        //lWPs[i]->Pp = lWPs[i]->MPU - lWPs[i]->OZMPUp; //Deprecated
        if(i == 0) lWPs[i]->Pv = 0.0;
        else lWPs[i]->Pv = LMATH::calc_fork(wp1.latlon.x, wp1.latlon.y, 0, a.x, a.y, 0, dat);
        lWPs[i]->Pp = LMATH::calc_fork(wp2.latlon.x, wp2.latlon.y, 0, a.x, a.y, 0, dat);

        //Distance calculation
        lWPs[i]->Spas = Spas;
        lWPs[i]->S = LMATH::calc_distance(a.x, a.y, b.x, b.y);
        Spas+= lWPs[i]->S;
        lWPs[i]->LPT = 5;	//TODO Probably turndistance, and I guess maybe S will be changed depending on this value?
	}//for
	
    lWPs[i]->S = 0;
    lWPs[i]->Spas = Spas;
    lWPs[i]->Srem = 0;

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
*/
