#include "MathTool.h"

bool FloatEqual(float a, float b)
{
	auto res = std::abs(a - b) < 1e-6;
	return res;
};

float Deg2Rad(float deg)
{
	return deg / 180.0f * PI;
}

float Rad2Deg(float rad)
{
	return rad / PI * 180.0f;
}