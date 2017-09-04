#ifndef NVUPOINT_H
#define NVUPOINT_H

#include <waypoint.h>

class NVUPOINT: public WAYPOINT
{
    private:
        static void swap(NVUPOINT& m, const NVUPOINT& wp);
        WAYPOINT* rsbn = NULL;

    public:
        double OZMPUv = 0;
        double OZMPUp = 0;
        double S = 0;
        double LPT = 0;
        double Pv = 0;
        double Pp = 0;
        double MPU = 0;
        double IPU = 0;
        double Spas = 0;
        double Srem = 0;


        double Sm = 0;
        double Zm = 0;
        double MapAngle = 0;
        double Atrg = 0;
        double Dtrg = 0;

        NVUPOINT();
        NVUPOINT(const NVUPOINT& wp);
        NVUPOINT(const WAYPOINT& wp);
        virtual ~NVUPOINT();
        NVUPOINT* clone();

        WAYPOINT* getRSBN() const;
        void setRSBN(const WAYPOINT *_rsbn);
        void calc_rsbn_corr(CPoint p2R);
};
#endif // NVUPOINT_H
