#pragma once

#include "crazygaze/micromuc/czmicromuc.h"

namespace cz
{

inline bool isNearlyEqual(float a, float b, float errorTolerance = 1.e-8f)
{
	return abs(a-b) <= errorTolerance;
}
	
} // namespace cz

