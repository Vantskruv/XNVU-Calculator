/*
 * CPoint.h
 *
 *  Created on: 30 mar 2011
 *      Author: johannes
 */

#ifndef CPOINT_H_
#define CPOINT_H_

//#include <allegro5/allegro.h>
#include <string>


class CPoint
{

	public:
		double x, y, z;

		CPoint& operator=(const CPoint&);
		CPoint& operator+=(const CPoint&);
		CPoint& operator-=(const CPoint&);
		CPoint& operator*=(const CPoint&);
		CPoint& operator/=(const CPoint&);
		CPoint& operator*=(double);
		CPoint& operator/=(double);

		const CPoint operator+(const CPoint&) const;
		const CPoint operator-(const CPoint&) const;
		const CPoint operator*(const CPoint&) const;
		const CPoint operator/(const CPoint&) const;
		const CPoint operator*(double) const;
		const CPoint operator/(double) const;
		const CPoint cross(const CPoint&) const;
		double dot(const CPoint&) const;
		
		std::string getString() const;
		CPoint getInversed() const;
		double getLength() const;
		CPoint getNormal(bool) const;                 //Not normalized.
		CPoint getNormalized() const;
		CPoint getRotated(double) const;              //Argument is in radians.
		double getAngle(const CPoint&) const;		//Get angle in radians between vectors. Angle returned is between 0 - 360 degrees.
		bool isSimilar(const CPoint&, double margin=0.0001) const; //Point to compare and margin

		CPoint();
		CPoint(const CPoint&);
		CPoint(double, double, double);
		CPoint(int, int, int);
		virtual ~CPoint();
};


#endif /* CPOINT_H_ */
