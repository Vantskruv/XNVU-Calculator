/*
 * CPoint.cpp
 *
 *  Created on: 30 mar 2011
 *      Author: johannes
 */

#include "CPoint.h"
#include <cmath>
#include <sstream>
#include <iomanip>

CPoint::CPoint(const CPoint& _p)
{
	x = _p.x;
	y = _p.y;
	z = _p.z;
}

CPoint& CPoint::operator=(const CPoint& _p)
{
	if(this == &_p) return *this;
	x = _p.x;
	y = _p.y;
	z = _p.z;

	return *this;
}

CPoint& CPoint::operator+=(const CPoint& _p)
{
	x += _p.x;
	y += _p.y;
	z += _p.z;
	return *this;	
}

CPoint& CPoint::operator-=(const CPoint& _p)
{
	x -= _p.x;
	y -= _p.y;
	z -= _p.z;

	return *this;
}

CPoint& CPoint::operator*=(const CPoint& _p)
{
	x *= _p.x;
	y *= _p.y;
	z *= _p.z;

	return *this;
}

CPoint& CPoint::operator/=(const CPoint& _p)
{
	x /= _p.x;
	y /= _p.y;
	z /= _p.z;

	return *this;
}

CPoint& CPoint::operator*=(double _d)
{
	x*=_d;
	y*=_d;
	z*=_d;

	return *this;
}

CPoint& CPoint::operator/=(double _d)
{
	x/=_d;
	y/=_d;
	z/=_d;

	return *this;
}




const CPoint CPoint::operator+(const CPoint& _p) const
{
	CPoint r = *this;
	r+=_p;
	return r;
}

const CPoint CPoint::operator-(const CPoint& _p) const
{
	CPoint r = *this;
	r-=_p;
	return r;
}

const CPoint CPoint::operator*(const CPoint& _p) const
{
	CPoint r = *this;
	r*=_p;
	return r;
}

const CPoint CPoint::operator/(const CPoint& _p) const
{
	CPoint r = *this;
	r/=_p;
	return r;
}

const CPoint CPoint::operator*(double _d) const
{
	CPoint r = *this;
	r*=_d;

	return r;
}

const CPoint CPoint::operator/(double _d) const
{
	CPoint r = *this;
	r/=_d;

	return r;
}

double CPoint::dot(const CPoint& _p) const
{
	return x*_p.x + y*_p.y + z*_p.z;
}

const CPoint CPoint::cross(const CPoint& _p) const
{
	//x = a2*b3 - a3*b2
	//y = a3*b1 - a1*b3
	//z = a1*b2 - a2*b1
	return CPoint(y*_p.z - z*_p.y, z*_p.x - x*_p.z, x*_p.y - y*_p.x);
}

std::string CPoint::getString() const
{
	std::stringstream ss;
	ss << std::setprecision(5) << "(" << x << ", " << y << ")"; //", " << z <<")";
	std::string rs = ss.str();
	
	return rs;
}

CPoint CPoint::getInversed() const
{
	return CPoint(-x, -y, -z);
}

double CPoint::getLength() const
{
	/*
	al_fixed fx = al_ftofix(x);
	al_fixed fy = al_ftofix(y);
	al_fixed fz = al_ftofix(z);

	al_fixed length = al_fixmul(fx, fx) + al_fixmul(fy, fy) + al_fixmul(fz, fz);
	length = al_fixsqrt(length);

	return al_fixtof(length);
	//return al_fixtof(al_fixhypot(al_ftofix(x), al_ftofix(y)));
	*/


	//double d = x*x + y*y + z*z;
	//d = sqrt(d);

	return sqrt(x*x + y*y + z*z);
}

CPoint CPoint::getNormal(bool _right) const
{
	CPoint nV;

	if(_right)
	{
		nV.x = -y;
		nV.y = x;
	}//if
	else
	{
		nV.x = y;
		nV.y = -x;
	}

	return nV;
}

CPoint CPoint::getNormalized() const
{

	return *this / getLength();
}

CPoint CPoint::getRotated(double _theta) const	//TODO Currently ALLEGRO dependent
{
	//ALLEGRO_TRANSFORM tr;
	//al_identity_transform(&tr);
	//al_rotate_transform(&tr, _theta);

	float _x = x;
	float _y = y;
	//al_transform_coordinates(&tr, &_x, &_y);

	return CPoint(_x, _y, 0.0);
}

double CPoint ::getAngle(const CPoint& _p) const
{
	return atan2(_p.x, _p.y) - atan2(x, y);
}

bool CPoint::isSimilar(const CPoint& _p, double margin) const
{
/*
	int xa = round(x);
	int ya = round(y);
	int za = round(z);
	int xb = round(_p.x);
	int yb = round(_p.y);
	int zb = round(_p.z);
	
	if(xa==xb && ya==yb && za==zb) return true;

	return false;
*/

	double dxa = fabs(_p.x - x);
	double dya = fabs(_p.y - y);
	double dza = fabs(_p.z - z);

	if(dxa>margin || dya>margin || dza>margin) return false;

	return true;

}

CPoint::CPoint()
{
	x = 0;
	z = 0;
	y = 0;
}

/*
CPoint::CPoint(al_fixed _x, al_fixed _y, al_fixed _z)
{
	x = _x;
	y = _y;
	z = _z;
}
*/

CPoint::CPoint(double _x, double _y, double _z)
{
	x = _x;
	y = _y;
	z = _z;
}

CPoint::CPoint(int _x, int _y, int _z)
{
	x = _x;
	y = _y;
	z = _z;
}

CPoint::~CPoint()
{
}
