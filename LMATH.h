#ifndef LMATH_HEADER
#define LMATH_HEADER

#include <CPoint.h>



namespace LMATH
{

    static const double feetToMeter(double f){ return 0.3048*f;}
    static const double meterToFeet(double f){ return f/0.3048;}
    static const int KM_RADIUS = 6371.009;
	bool GetLineIntersection(const CPoint& A, const CPoint & B, const CPoint& C, const CPoint& D, double& mua);
	double GetClosestPointOnVector(const CPoint& _p, const CPoint& _v);
	CPoint latlonToVector(const CPoint& p);
	CPoint vectorToLatlon(const CPoint& p);
	CPoint bearingToVector(const CPoint& p, const double& b);
    double calc_fork(double lat1, double lon1, int h1, double lat2, double lon2, int h2, long dat);
    double calc_bearing(const CPoint& _a, const CPoint& _b);
    double bearingTo(const CPoint&, const CPoint&);
    double finalBearingTo(const CPoint&, const CPoint&);
    double angleTo360(double);
    double calc_bearing_radial(const CPoint& a, const CPoint& b );
    double calc_distance(double lat1, double lon1, double lat2, double lon2);
    double calc_distance(const CPoint&, const CPoint&);
    void calc_destination_orthodromic(double lat1, double lon1, double brng, double dt, double& lat2, double& lon2);
    void calc_destination_orthodromic(CPoint a, double brng, double dt, CPoint &b);
    void calc_destination_rhumb(CPoint a, double brng, double dt, CPoint& b);

}//namespace LMATH
#endif
