#ifndef NVU_GENERATE_HEADER
#define NVU_GENERATE_HEADER

#include <vector>
//#include "XFMS_DATA.h"
#include "waypoint.h"

class NVUPOINT: public WAYPOINT
{
    private:
        static void swap(NVUPOINT& m, const NVUPOINT& wp)
        {
            m.OZMPUv = wp.OZMPUv;
            m.OZMPUp = wp.OZMPUp;
            m.S = wp.S;
            m.LPT = wp.LPT;
            m.Pv = wp.Pv;
            m.Pp = wp.Pp;
            m.MPU = wp.MPU;
            m.IPU = wp.IPU;
            m.Spas = wp.Spas;
            m.Srem = wp.Srem;

            m.rsbn = wp.rsbn;
            m.Sm = wp.Sm;
            m.Zm = wp.Zm;
            m.MapAngle = wp.MapAngle;
            m.Atrg = wp.Atrg;
            m.Dtrg = wp.Dtrg;
        }

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
            swap(*this, wp);
        }

        NVUPOINT(const WAYPOINT& wp) : WAYPOINT(wp)
        {

        }

        NVUPOINT* clone()
        {
            return new NVUPOINT(*this);
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
