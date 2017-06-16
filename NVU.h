#ifndef NVU_GENERATE_HEADER
#define NVU_GENERATE_HEADER

#include <vector>
//#include "XFMS_DATA.h"
#include "waypoint.h"

class NVUPOINT: public WAYPOINT
{
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

        WAYPOINT* rsbn = NULL;
        double Sm = 0;
        double Zm = 0;
        double MapAngle = 0;
        double Atrg = 0;
        double Dtrg = 0;

        NVUPOINT(): WAYPOINT(){}
        NVUPOINT(const NVUPOINT& wp) : WAYPOINT(wp)
		{
            OZMPUv = wp.OZMPUv;
            OZMPUp = wp.OZMPUp;
            S = wp.S;
            LPT = wp.LPT;
            Pv = wp.Pv;
            Pp = wp.Pp;
            MPU = wp.MPU;
            IPU = wp.IPU;
            Spas = wp.Spas;
            Srem = wp.Srem;

            rsbn = wp.rsbn;
            Sm = wp.Sm;
            Zm = wp.Zm;
            MapAngle = wp.MapAngle;
            Atrg = wp.Atrg;
            Dtrg = wp.Dtrg;
        }

        NVUPOINT(const WAYPOINT& wp) : WAYPOINT(wp)
        {

        }

/*
        void clone(const NVUPOINT& wp)
        {
            latlon = wp.latlon;
            type = wp.type;
            name = wp.name;
            name2 = wp.name2;
            range = wp.range;
            freq = wp.freq;
            alt = wp.alt;
            elev = wp.elev;
            trans_alt = wp.trans_alt;
            trans_level = wp.trans_level;
            longest_runway = wp.longest_runway;
            country = wp.country;
            MD = wp.MD;
            ADEV = wp.ADEV;
            wpOrigin = wp.wpOrigin;
            rsbn = wp.rsbn;
        }
*/

        void calc_rsbn_corr(CPoint p2R);
		
        virtual ~NVUPOINT(){}
};




class NVU
{
	public:
        //double NVU_FORK;
        //std::vector<NVUPOINT*> lWPs;
        static void generate(std::vector<NVUPOINT*>& wps, double &NVU_FORK, long dat);
        //void calc_rsbn_corr(NVUPOINT*, NVUPOINT*);    //Set RSBN for NVUPOINT and calculate correction
        //void clear();
        //void printWaypoints();
};
#endif
