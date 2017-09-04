#include "airway.h"

void AIRWAY::swap(AIRWAY& a, const AIRWAY& r)
{
    a.name = r.name;
    a.distance = r.distance;
    a.lATS.clear();
    for(unsigned int i=0; i<r.lATS.size(); i++) a.lATS.push_back(r.lATS[i]);
}

AIRWAY::AIRWAY()
{
    name = "";
    distance = -1;
}

AIRWAY::AIRWAY(const AIRWAY& awy)
{
    swap(*this, awy);
}

AIRWAY& AIRWAY::operator=(const AIRWAY& other)
{
    swap(*this, other);
    return *this;
}

AIRWAY* AIRWAY::clone()
{
    return new AIRWAY(*this);
}
