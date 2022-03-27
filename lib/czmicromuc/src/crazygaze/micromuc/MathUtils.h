#pragma once

#include "crazygaze/micromuc/czmicromuc.h"

namespace cz
{

inline bool isNearlyEqual(float a, float b, float errorTolerance = 1.e-8f)
{
	return abs(a-b) <= errorTolerance;
}

template<typename T>
T clamp(T value, T min, T max)
{
	if (value < min)
		return min;
	else if (value > max)
		return max;
	else
		return value;
}
	
} // namespace cz

