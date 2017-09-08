#include "LMATH.h"
#include <cmath>
#include <coremag.h>

#define KM_RADIUS 6371.009


double LMATH::IAS_to_MACH(double CAS, double FL, double ISA)
{
    /*
    const double EARTH_RADIUS = 6356.766; //KM
    FL = (EARTH_RADIUS*FL) / (EARTH_RADIUS + FL);
    const double G = 9.80665; //Earth gravitation
    const double M = 0.0289644; //Molar mass of dry air
    const double R = 0.0083144598; //Gas constant
    const double LP = -6.5;     //Laps rate (6.5C/KM)
    const double A0 = 340.3; //Speed of sound at sea level (m/s)
    const double D0 = 1.2250; //Sea standard air density (kg/m3)

    //Temperatures at sea level and altitude
    double T0 = 273.15 + 15 + ISA; //Sea standard temperature (15 degrees) + ISA
    double T = T0 + LP*FL;       //OAT temp (6.5 degree change per 1000 meter);

    //Calculate density of air (kg/m3)
    double DA = D0 * pow(T0/T,  (1.0 + (G*M)/(R*LP)));

    //Convert CAS to m/s from km/h
    CAS = CAS/3.6;


    double q = 0.5*DA*TAS*TAS;	//Dynamic pressure
    double qc = q*pow(_MACH + 1.0, 2.0/7.0); //Impact pressure
    double CAS = A0*sqrt(5.0*(pow((qc/P0) + 1.0, 2.0/7.0) - 1.0)) + K;
    _MACH = pow( P0*(pow(pow(CAS/A0, 2.0)/5.0 + 1.0, 7.0/2.0) - 1.0)/q, 7.0/2.0) - 1.0;


    double Q = 0.5*DA*TAS*TAS;	//Dynamic pressure
    double QC = Q*(1.0 + pow(_MACH, 2.0)/4.0 + pow(_MACH, 4.0)/40.0 + pow(_MACH, 6.0)/1600.0); //Impact pressure
    double CAS = A0*sqrt(5.0*(pow((QC/P0) + 1.0, 2.0/7.0) - 1.0)) + K;
    */

    return 0.0;


    //CAS = A*sqrt(5*((((0.5*D*(A*M*sqrt(T/t))²*(1 + M²/4 + M⁴/40 + M⁶/1600)  / p) + 1)² - 1))

}

double LMATH::MACH_to_IAS(double MACH, double FL, double ISA)
{
    /*
    const double EARTH_RADIUS = 6356.766; //KM
    FL = (EARTH_RADIUS*FL) / (EARTH_RADIUS + FL);
    const double P0 = 101325; //Sea standard pressure
    const double G = 9.80665; //Earth gravitation
    const double M = 0.0289644; //Molar mass of dry air
    const double R = 0.0083144598; //Gas constant
    const double LP = -6.5;     //Laps rate (6.5C/KM)
    const double A0 = 340.3; //Speed of sound at sea level (m/s)
    const double D0 = 1.2250; //Sea standard air density (kg/m3)
    const double K = 0.0; //Margin of error

    //Temperatures at sea level and altitude
    double T0 = 273.15 + 15 + ISA; //Sea standard temperature (15 degrees) + ISA
    double T = T0 + LP*FL;       //OAT temp (6.5 degree change per 1000 meter);

    //Calculate density of air (kg/m3) (affected by ISA and flightlevel)
    double DA = D0 * pow(T0/T,  (1.0 + (G*M)/(R*LP)));

    //Calculate TAS(m/s) from MACH
    double TAS = A0*MACH*sqrt(T/T0);

    //Calculate CAS (it seems this is what the airspeed indicator shows...) (affected by ISA, flightlevel and MACH speed)
    double Q = 0.5*DA*TAS*TAS;	//Dynamic pressure
    double QC = Q*(1.0 + pow(_MACH, 2.0)/4.0 + pow(_MACH, 4.0)/40.0 + pow(_MACH, 6.0)/1600.0); //Impact pressure
    double CAS = A0*sqrt(5.0*(pow((QC/P0) + 1.0, 2.0/7.0) - 1.0)) + K;

    return CAS;
    */

    return 0.0;

    //Calculate IAS(m/s) from TAS. Equation is actually for EAS, but IAS is the same as EAS if there is a 0 error.
    //return TAS*sqrt(DA/D0)*3.6;

/*
    S = A*M*sqrt(T/t);
    Q = 0.5*D*S^2
    q = Q*(1 + (M^2)/4 + (M^4)/40 + (M^6)/1600)
    C = A*sqrt(5*( (q/P)+1)^(2/7) - 1)


    Q = 0.5*D*(A*M)^2*T/t
    q = Q*(1 + (M^2)/4 + (M^4)/40 + (M^6)/1600)
    C = A*sqrt(5*( (q/P)+1)^(2/7) - 1)

    q = 0.5*D*(A*M)^2*T*(1 + (M^2)/4 + (M^4)/40 + (M^6)/1600)/t
    C = A*sqrt(5*( (q/P)+1)^(2/7) - 1)

    */

}

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

//Converting lat/lon coordinates to ???????? But it looks flat and nice on screen
void LMATH::latlonToScreen(CPoint& _p)
{
   CPoint p = _p;
   p = p*M_PI/180.0;
   //WORKS NICLEY!!! SIMPLE CODE!!! 536 MILLION HOURS TO SUCCEED!
   p.y = p.y;
   p.x = log(tan(M_PI*0.25 + p.x*0.5));
   //ROTATION!
   _p.x = p.y;
   _p.y = -p.x;
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


/*
double LMATH::calc_fork(double lat1, double lon1, int h1, double lat2, double lon2, int h2, long dat)
{
    double m1 = calc_magvar(lat1, lon1, dat, double(h1)/1000.0);
    double m2 = calc_magvar(lat2, lon2, dat, double(h2)/1000.0);
    double lat = (lat1 + lat2)*0.5*M_PI/180.0;
    return m1 - m2  + (lon2 - lon1)*sin(lat);
}
*/

double LMATH::bearingTo(const CPoint& dep, const CPoint& arr)
{
    double lat1 = dep.x*M_PI/180.0;
    double lat2 = arr.x*M_PI/180.0;
    double dlon = (dep.y - arr.y)*M_PI/180.0;

    double y = sin(dlon)*cos(lat2);
    double x = cos(lat1)*sin(lat2) - sin(lat1)*cos(lat2)*cos(dlon);
    double a = std::remainder((atan2(y, x)*180.0/M_PI) + 360.0, 360.0);

    return a;


    /*
    LatLon.prototype.bearingTo = function(point) {
    if (!(point instanceof LatLon)) throw new TypeError('point is not LatLon object');

    var φ1 = this.lat.toRadians(), φ2 = point.lat.toRadians();
    var Δλ = (point.lon-this.lon).toRadians();

    // see http://mathforum.org/library/drmath/view/55417.html
    var y = Math.sin(Δλ) * Math.cos(φ2);
    var x = Math.cos(φ1)*Math.sin(φ2) -
            Math.sin(φ1)*Math.cos(φ2)*Math.cos(Δλ);
    var θ = Math.atan2(y, x);

    return (θ.toDegrees()+360) % 360;
    */
};

double LMATH::finalBearingTo(const CPoint& dep, const CPoint& arr)
{
    return std::remainder(LMATH::bearingTo(dep, arr) + 180.0, 360.0);
/*
LatLon.prototype.finalBearingTo = function(point) {
    if (!(point instanceof LatLon)) throw new TypeError('point is not LatLon object');

    // get initial bearing from destination point to this point & reverse it by adding 180°
    return ( point.bearingTo(this)+180 ) % 360;
};
*/

}

double LMATH::calc_fork(double lat1, double lon1, int h1, double lat2, double lon2, int h2, long dat)
{
    if(CPoint(lat1, lon1, 0.0).isSimilar(CPoint(lat2, lon2, 0.0), 0.0001)) return 0.0;

    double m1 = calc_magvar(lat1, lon1, dat, double(h1)/1000.0);
    double m2 = calc_magvar(lat2, lon2, dat, double(h2)/1000.0);

    CPoint dep(lat1, lon1, 0.0);
    CPoint arr(lat2, lon2, 0.0);

    double to = calc_bearing(dep, arr);
    double fr = std::remainder(calc_bearing(arr, dep) + 180.0, 360.0);

    double fork = m1 - m2  + fr - to;
    fork = std::remainder(fork, 360.0);

    return fork;
}


/*
double LMATH::calc_fork(double lat1, double lon1, int h1, double lat2, double lon2, int h2, long dat)
{
    double m1 = calc_magvar(lat1, lon1, dat, double(h1)/1000.0);
    double m2 = calc_magvar(lat2, lon2, dat, double(h2)/1000.0);

    lat1*=M_PI/180.0;
    lon1*=M_PI/180.0;
    lat2*=M_PI/180.0;
    lon2*=M_PI/180.0;
    double crs12, crs21;

    double d = acos(sin(lat1)*sin(lat2)+cos(lat1)*cos(lat2)*cos(lon1-lon2));
    if ((d==0.0) || (lat1==-(M_PI/180)*90.0))
    {
       crs12=2*M_PI;
    }
    else if (lat1==(M_PI/180)*90.0)
    {
       crs12=M_PI;
    }
    else
    {
       double argacos = (sin(lat2)-sin(lat1)*cos(d))/(sin(d)*cos(lat1));
       if (sin(lon2-lon1) < 0)
       {
         crs12=acosf(argacos);
       }
       else
       {
         crs12=2*M_PI-acosf(argacos);
       }
    };

    if ((d==0.0) || (lat2==-(M_PI/180)*90.0))
    {
       crs21=0.0;
    }
    else if(lat2==(M_PI/180)*90.0)
    {
       crs21=M_PI;
    }
    else
    {
       double argacos=(sin(lat1)-sin(lat2)*cos(d))/(sin(d)*cos(lat2));
       if(sin(lon1-lon2)<0)
       {
         crs21 = acosf(argacos);
       }
       else
       {
         crs21 = 2*M_PI-acosf(argacos);
       }
    }

    return (crs12 - crs21)*180/M_PI;


}
*/

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

void LMATH::calc_destination_orthodromic(double lat1, double lon1, double brng, double dt, double& lat2, double& lon2)
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

void LMATH::calc_destination_orthodromic(CPoint a, double brng, double dt, CPoint& b)
{
    a.x*=M_PI/180.0;
    a.y*=M_PI/180.0;
    brng*=M_PI/180.0;

    double r = KM_RADIUS;
    double lat3 = asin(sin(a.x) * cos(dt / r) + cos(a.x) * sin(dt / r) * cos(brng));
    double lon3 = a.y + atan2(sin(brng) * sin(dt / r) * cos(a.x) , cos(dt / r) - sin(a.x) * sin(lat3));

    b.x = lat3*180.0/M_PI;
    b.y = lon3*180.0/M_PI;
}

void LMATH::calc_destination_rhumb(CPoint a, double brng, double dt, CPoint& b)
{

    /*
    var Δφ = δ*Math.cos(θ);
    var φ2 = φ1 + Δλ;

    var Δψ = Math.log(Math.tan(φ2/2+Math.PI/4)/Math.tan(φ1/2+Math.PI/4));
    var q = Math.abs(Δψ) > 10e-12 ? Δφ / Δψ : Math.cos(φ1); // E-W course becomes ill-conditioned with 0/0

    var Δλ = δ*Math.sin(θ)/q;
    var λ2 = λ1 + Δλ;

    // check for some daft bugger going past the pole, normalise latitude if so
    if (Math.abs(φ2) > Math.PI/2) φ2 = φ2>0 ? Math.PI-φ2 : -Math.PI-φ2;
    */

    a.x*=M_PI/180.0;
    a.y*=M_PI/180.0;
    brng*=M_PI/180.0;

    double da = dt / KM_RADIUS; // angular distance in radians
    double dlat = da * cos(brng);
    double lat2 = a.x + dlat;

    // check for some daft bugger going past the pole, normalise latitude if so
    if(fabs(lat2) > M_PI/2.0) lat2 = lat2>0 ? M_PI-lat2 : -M_PI-lat2;

    double dd = log(tan(lat2/2.0+M_PI/4.0)/tan(a.x/2.0+M_PI/4.0));
    double q = fabs(dd) > 0 ? dlat / dd : cos(a.x); // E-W course becomes ill-conditioned with 0/0

    double dlon = da*sin(brng)/q;
    double lon2 = a.y + dlon;

    b.x = lat2*180/M_PI;
    b.y = std::remainder((lon2*180/M_PI) + 540, 360) - 180;

    /*


    a.x*=M_PI/180.0;
    a.y*=M_PI/180.0;
    brng*=M_PI/180.0;

    double da = dt/KM_RADIUS;
    double dlat = da*cos(brng);
    double lat2 = a.x + dlat;

    if(fabs(lat2)>(M_PI/2.0)) lat2 = (lat2>0 ? M_PI - lat2 : -M_PI - lat2);

    double dlon = log(tan(lat2/2.0 + M_PI/4.0)/tan(a.x/2.0 + M_PI/4.0));
    double q = (fabs(dlon) > 0 ? dlat/dlon : cos(a.x));

    double dd = da*sin(brng)/q;
    double lon2 = a.y + dd;

    b.x = lat2*180.0/M_PI;
    b.y = std::remainder((lon2*180.0/M_PI) + 540, 360) - 180;
    */
}
