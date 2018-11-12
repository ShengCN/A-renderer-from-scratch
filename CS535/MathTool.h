#pragma once
#include <algorithm>
#include "V3.h"
#include <random>

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

bool Side2D(V3 p, V3 v1, V3 v2, V3 v3);

bool IsInsideTriangle2D(V3 p, V3 v1, V3 v2, V3 v3);

V3 random_in_unit_shpere();
V3 random_in_unit_disk();

// source: rayctracing in a weekend
bool refract(V3 v, V3 n, float ni_over_nt, V3 &refracted);

float schlick(float cosin, float ref_idx);

template <typename T>
T Random(T low, T high)
{
	auto dv = std::random_device();
	std::mt19937 mt(dv());
	std::uniform_real_distribution<T> dist(low, high);
	return dist(mt);
}