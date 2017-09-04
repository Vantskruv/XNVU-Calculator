#ifndef AIRPORT_DATA_H
#define AIRPORT_DATA_H
#include <QString>
#include <vector>
#include <waypoint.h>

class AIRPORT_DATA
{
    private:
        static void swap(AIRPORT_DATA& a, const AIRPORT_DATA& r);

    public:
        QString city;                                 //The city airport belongs to
        std::vector<WAYPOINT*> lRunways;              //List of runways belonging to airport
        std::vector<std::pair<int, int> > lFreq;

        AIRPORT_DATA();
        AIRPORT_DATA(const AIRPORT_DATA& ad);
        AIRPORT_DATA& operator=(const AIRPORT_DATA& other);
        AIRPORT_DATA* clone();
        ~AIRPORT_DATA();
};

#endif // AIRPORT_DATA_H
