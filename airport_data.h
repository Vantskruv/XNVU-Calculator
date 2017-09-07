#ifndef AIRPORT_DATA_H
#define AIRPORT_DATA_H
#include <QString>
#include <vector>
#include <waypoint.h>

#define ATC_ATIS	50
#define ATC_UNICOM	51
#define ATC_DEL		52
#define ATC_GND		53
#define ATC_TWR		54
#define ATC_APP		55
#define ATC_DEP		56

class AIRPORT_DATA
{
    private:
        static void swap(AIRPORT_DATA& a, const AIRPORT_DATA& r);

    public:
        QString city;                                 //The city airport belongs to
        std::vector<WAYPOINT*> lRunways;              //List of runways belonging to airport
        std::vector<std::pair<int, int> > lFreq;

        static QString getFreqTypeStr(int _freqType);
        std::vector<std::pair<QString, QString> > getFreqs();

        AIRPORT_DATA();
        AIRPORT_DATA(const AIRPORT_DATA& ad);
        AIRPORT_DATA& operator=(const AIRPORT_DATA& other);
        AIRPORT_DATA* clone();
        ~AIRPORT_DATA();
};

#endif // AIRPORT_DATA_H
