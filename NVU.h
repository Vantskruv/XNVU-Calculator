#ifndef NVU_GENERATE_HEADER
#define NVU_GENERATE_HEADER

#include <vector>
#include "nvupoint.h"

class NVU
{
	public:
        static void generate(std::vector<NVUPOINT*>& wps, double &NVU_FORK, long dat);
};
#endif
