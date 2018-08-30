#include "MathTool.h"

bool FloatEqual(float a, float b)
{
	return std::abs(a - b) < 1e-7;
};

float Deg2Rad(float deg)
{
	return deg / 180.0f * PI;
}

float Rad2Deg(float rad)
{
	return rad / PI * 180.0f;
}