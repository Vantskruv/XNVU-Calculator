#include <nvupoint.h>
#include <LMATH.h>
#include <coremag.h>

void NVUPOINT::swap(NVUPOINT& m, const NVUPOINT& wp)
{
    m.OZMPUv = wp.OZMPUv;
    m.OZMPUp = wp.OZMPUp;
    m.S = wp.S;
    m.LPT = wp.LPT;
    m.Pv = wp.Pv;
    m.Pp = wp.Pp;
    m.MPU = wp.MPU;
    m.IPU = wp.IPU;
    m.Spas = wp.Spas;
    m.Srem = wp.Srem;

    m.rsbn = (wp.rsbn ? new WAYPOINT(*wp.rsbn) : NULL);
    m.Sm = wp.Sm;
    m.Zm = wp.Zm;
    m.MapAngle = wp.MapAngle;
    m.Atrg = wp.Atrg;
    m.Dtrg = wp.Dtrg;
}
NVUPOINT::NVUPOINT(): WAYPOINT(){}
NVUPOINT::NVUPOINT(const NVUPOINT& wp) : WAYPOINT(wp)
{
    swap(*this, wp);
}
NVUPOINT::NVUPOINT(const WAYPOINT& wp) : WAYPOINT(wp)
{

}
NVUPOINT* NVUPOINT::clone()
{
    return new NVUPOINT(*this);
}
NVUPOINT::~NVUPOINT()
{
    if(rsbn) delete rsbn;
}


WAYPOINT* NVUPOINT::getRSBN() const{ return rsbn; }

void NVUPOINT::setRSBN(const WAYPOINT* _rsbn)
{
    if(rsbn) delete rsbn;
    if(_rsbn) rsbn = new WAYPOINT(*_rsbn);
    else rsbn = NULL;
}

void NVUPOINT::calc_rsbn_corr(CPoint _p2R)
{
    if(rsbn==NULL) return;

    //Set coordinates to radial
    CPoint pRR = rsbn->latlon*M_PI/180.0;
    CPoint p1R = latlon*M_PI/180.0;
    CPoint p2R = _p2R*M_PI/180.0;

    //Calculate point on course which is in 90 degree angle to the RSBN (vI)
    CPoint v = LMATH::latlonToVector(p2R) - LMATH::latlonToVector(p1R);                         //Vector v pointing from p1R to p2R
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
    vI = LMATH::latlonToVector(p2R) - LMATH::latlonToVector(p1R) + LMATH::latlonToVector(pRR);
    vI = LMATH::vectorToLatlon(vI);
    vI = vI*180.0/M_PI;
    MapAngle = LMATH::calc_bearing(rsbn->latlon, vI);       //YK - A????
    if(rsbn->wpOrigin == WAYPOINT::ORIGIN_X10_EARTHNAV || WAYPOINT::ORIGIN_X11_CUSTOM_EARTHNAV || WAYPOINT::ORIGIN_X11_DEFAULT_EARTHNAV) MapAngle-=rsbn->ADEV;

    //Calculate Atrg
    Atrg = LMATH::calc_bearing(rsbn->latlon, _p2R);

    //Calculate Dtrg
    Dtrg = LMATH::calc_distance(rsbn->latlon, _p2R);
}
