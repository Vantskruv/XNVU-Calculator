#ifndef AIRWAY_H
#define AIRWAY_H

#include <waypoint.h>

class AIRWAY
{
    private:
        static void swap(AIRWAY& a, const AIRWAY& r);

    public:
        QString name;
        double distance;
        std::vector<WAYPOINT*> lATS;    //Waypoints are not deleted when removing this airway, as they already are contained and manager in the lWP list.

        AIRWAY();
        AIRWAY(const AIRWAY& awy);
        AIRWAY& operator=(const AIRWAY& other);
        AIRWAY* clone();

};
#endif // AIRWAY_H
