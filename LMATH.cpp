#include "LMATH.h"
#include <cmath>
#include <coremag.h>

#define KM_RADIUS 6371.009

bool LMATH::GetLineIntersection(const CPoint& A, const CPoint & B, const CPoint& C, const CPoint& D, double& mua)
{
	CPoint lA[] = {A, B};
	CPoint lB[] = {C, D};


	double d1343 = (lB[0] - lA[0]).dot(lB[0] - lB[1]);
	double d4321 = (lB[0] - lB[1]).dot(lA[0] - lA[1]);
	double d1321 = (lB[0] - lA[0]).dot(lA[0] - lA[1]);
	double d2121 = (lA[0] - lA[1]).dot(lA[0] - lA[1]);
	double d4343 = (lB[0] - lB[1]).dot(lB[0] - lB[1]);
	double div = d2121*d4343 - d4321*d4321;
	if(div == 0) return false;
	mua = (d1343*d4321 - d1321*d4343) / (d2121*d4343 - d4321*d4321);

	return true;
}

//p is any point relative to vector, v is the vector. Returns the point on vector closest to p
//I.e. what is the nearest point one line A-B to point P?
//d = GetClosestPointOnVector(P - A, B - A)
//PC = A + (B - A).getNormalized() * d
double LMATH::GetClosestPointOnVector(const CPoint& _p, const CPoint& _v)
{
    double vDot = _v.dot(_v);
    double pvDot = _p.dot(_v);
    double t = pvDot / vDot;

    return t;
}

CPoint LMATH::latlonToVector(const CPoint& p)
{
	/*
		From lat/lon to n-vector
		
		v(x,y,z) = [ cos(lat)*cos(lon), cos(lat)*sin(lon), sin(lat) ]

	*/

	return CPoint(cos(p.x)*cos(p.y), cos(p.x)*sin(p.y), sin(p.x));
}

CPoint LMATH::vectorToLatlon(const CPoint& p)
{
/*
		From n-vector to lat/lon
		lat = atan2(vz, sqrt(vx² + vy²))
		lon = atan2(vy, vx)
*/
	return CPoint(atan2(p.z, sqrt(p.x*p.x + p.y*p.y)), atan2(p.y, p.x), 0.0);
}

CPoint LMATH::bearingToVector(const CPoint& p, const double& b)
{
	return CPoint(
				 sin(p.y)*cos(b) - sin(p.x)*cos(p.y)*sin(b),
				-cos(p.y)*cos(b) - sin(p.x)*sin(p.y)*sin(b),
				 cos(p.x)*sin(b)
			  );
}


double LMATH::calc_fork(double lat1, double lon1, double lat2, double lon2, long dat)
{
    double m1 = calc_magvar(lat1, lon1, dat);
    double m2 = calc_magvar(lat2, lon2, dat);
    double lat = (lat1 + lat2)*0.5*M_PI/180.0;

    return m1 - m2  + (lon2 - lon1)*sin(lat);
}

double LMATH::calc_bearing(const CPoint& _a, const CPoint& _b)
{
    CPoint a = _a*M_PI/180.0;
    CPoint b = _b*M_PI/180.0;

    double brng = calc_bearing_radial(a, b);
    brng*=180/M_PI;

    return angleTo360(brng);
}

double LMATH::calc_bearing_radial(const CPoint& a, const CPoint& b )
{
    return atan2(sin(b.y - a.y)*cos(b.x), cos(a.x)*sin(b.x) - sin(a.x)*cos(b.x)*cos(b.y - a.y));
}

double LMATH::angleTo360(double _a)
{
    _a = std::remainder(_a, 360.0);
    return (_a<0) ? _a + 360 : _a;
}


double LMATH::calc_distance(double lat1, double lon1, double lat2, double lon2)
{
    lat1*=M_PI/180.0;
    lon1*=M_PI/180.0;
    lat2*=M_PI/180.0;
    lon2*=M_PI/180.0;

    double londelta = lon2 - lon1;
    double a = pow(cos(lat2) * sin(londelta), 2) + pow(cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2)*cos(londelta), 2);
    double b = sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(londelta);
    double angle = atan2(sqrt(a), b);

    return angle*KM_RADIUS;
}

double LMATH::calc_distance(const CPoint& A, const CPoint& B)
{
    return calc_distance(A.x, A.y, B.x, B.y);
}

void LMATH::calc_destination(double lat1, double lon1, double brng, double dt, double& lat2, double& lon2)
{
    lat1*=M_PI/180.0;
    lon1*=M_PI/180.0;
    brng*=M_PI/180.0;

    double r = KM_RADIUS;
    double lat3 = asin(sin(lat1) * cos(dt / r) + cos(lat1) * sin(dt / r) * cos(brng));
    double lon3 = lon1 + atan2(sin(brng) * sin(dt / r) * cos(lat1) , cos(dt / r) - sin(lat1) * sin(lat3));

    lat2 = lat3*180.0/M_PI;
    lon2 = lon3*180.0/M_PI;
}

