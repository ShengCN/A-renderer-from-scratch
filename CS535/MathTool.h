#pragma once
#include <algorithm>

bool FloatEqual(float a, float b)
{
	return std::abs(a - b) < 1e-7;
};