#pragma once
#include <algorithm>

const float PI = 3.1415926f;

bool FloatEqual(float a, float b);
float Deg2Rad(float deg);
float Rad2Deg(float rad);

template <typename T>
T Clamp(T n, T low, T high)
{
	n = max(n, low);
	n = min(n, high);
	return n;
}

float Fract(float n);
