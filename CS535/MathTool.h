#pragma once
#include <algorithm>

class V3;
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

bool InsideTriangle(V3 p, V3 v1, V3 v2, V3 v3);
