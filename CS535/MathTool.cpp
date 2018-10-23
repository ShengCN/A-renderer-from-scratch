#include "MathTool.h"
#include "V3.h"

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
