#include "airport_data.h"
#include <waypoint.h>

void AIRPORT_DATA::swap(AIRPORT_DATA& a, const AIRPORT_DATA& r)
{
    //for(unsigned int i=0; i<a.lRunways.size(); i++) delete a.lRunways[i];
    //a.lRunways.clear();
    a.lFreq.clear();

    a.city = r.city;
    //for(unsigned int i=0; i<r.lRunways.size(); i++) a.lRunways.push_back(new WAYPOINT(*r.lRunways[i]));//new WAYPOINT(*r.lRunways[i]));
    for(unsigned int i=0; i<r.lFreq.size(); i++) a.lFreq.push_back(std::make_pair(r.lFreq[i].first, r.lFreq[i].second));
}


AIRPORT_DATA::AIRPORT_DATA()
{
    city = "";
}

AIRPORT_DATA::AIRPORT_DATA(const AIRPORT_DATA& ad)
{
    swap(*this, ad);
}

AIRPORT_DATA& AIRPORT_DATA::operator=(const AIRPORT_DATA& other)
{
    swap(*this, other);
    return *this;
}

AIRPORT_DATA* AIRPORT_DATA::clone()
{
    return new AIRPORT_DATA(*this);
}

AIRPORT_DATA::~AIRPORT_DATA()
{
    //for(unsigned int i=0; i<lRunways.size(); i++) delete lRunways[i];
}
