#include "MathTool.h"
#include "V3.h"
#include <random>

bool FloatEqual(float a, float b)
{
	auto res = std::abs(a - b) < 1e-5;
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

float Fract(float n)
{
	return n - static_cast<int>(n);
}

bool Side2D(V3 p, V3 v1, V3 v2, V3 v3)
{
	float x1 = v1[0], x2 = v2[0], y1 = v1[1], y2 = v2[1];

	V3 coeff(y2 - y1, x1 - x2, x2 * y1 - x1 * y2);

	float res1 = coeff * V3(p[0], p[1], 1.0f);
	float res2 = coeff * V3(v3[0], v3[1], 1.0f);

	return res1 * res2 >= 0.0f;
}

bool IsInsideTriangle2D(V3 p, V3 v1, V3 v2, V3 v3)
{
	bool s1, s2, s3;
	s1 = Side2D(p, v1, v2, v3);
	s2 = Side2D(p, v2, v3, v1);
	s3 = Side2D(p, v3, v1, v2);

	return s1 && s2 && s3;
}

V3 random_in_unit_shpere()
{
	V3 p(0.0f);
	do
	{
		p = V3(Random(0.0f, 1.0f), Random(0.0f, 1.0f), Random(0.0f, 1.0f)) * 2.0f - V3(1.0f);
	} while (p.Length() >= 1.0f);

	return p;
}

V3 random_in_unit_disk()
{
	V3 p(0.0f);
	do
	{
		p = V3(Random(0.0f, 1.0f), Random(0.0f, 1.0f), 0.0f) * 2.0f - V3(1.0f, 1.0f, 0.0f);
	} while (p.Length() >= 1.0f);
	return p;
}

bool refract(V3 v, V3 n, float ni_over_nt, V3& refracted)
{
	V3 uv = v.UnitVector();
	n = n.UnitVector();
	float dt = uv * n;
	float discriminant = 1.0f - ni_over_nt * ni_over_nt * (1.0f - dt * dt);
	if (discriminant > 0.0f)
	{
		refracted = (uv - n * dt) * ni_over_nt - n * sqrt(discriminant);
		return true;
	}
	return false;
}

float schlick(float cosin, float ref_idx)
{
	float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
	r0 = r0 * r0;
	return r0 + (1.0f - r0) * pow(1.0f - cosin, 5);
}
