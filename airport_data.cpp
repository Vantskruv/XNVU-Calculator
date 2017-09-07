#include "airport_data.h"
#include <waypoint.h>

void AIRPORT_DATA::swap(AIRPORT_DATA& a, const AIRPORT_DATA& r)
{
    for(unsigned int i=0; i<a.lRunways.size(); i++) delete a.lRunways[i];
    a.lRunways.clear();
    a.lFreq.clear();


    a.city = r.city;
    for(unsigned int i=0; i<r.lRunways.size(); i++) a.lRunways.push_back(new WAYPOINT(*r.lRunways[i]));
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
    for(unsigned int i=0; i<lRunways.size(); i++) delete lRunways[i];
}

QString AIRPORT_DATA::getFreqTypeStr(int _freqType)
{
    switch(_freqType)
    {
        case ATC_ATIS: return "Atis";
        case ATC_UNICOM: return "Unicom";
        case ATC_DEL: return "Delivery";
        case ATC_GND: return "Ground";
        case ATC_TWR: return "Tower";
        case ATC_APP: return "Approach";
        case ATC_DEP: return "Departure";
    }

    return "Unkknown";
}

std::vector< std::pair<QString, QString> > AIRPORT_DATA::getFreqs()
{
    std::vector< std::pair<QString, QString> > lFT;

    for(unsigned int i=0; i<lFreq.size(); i++)
    {
        QString sFreq = (lFreq[i].second>100000 ? QString::number(lFreq[i].second/1000.0, 'f', 3) : QString::number(lFreq[i].second/100.0, 'f', 2));
        lFT.push_back(std::make_pair(getFreqTypeStr(lFreq[i].first), sFreq));
    }

    return lFT;
}
