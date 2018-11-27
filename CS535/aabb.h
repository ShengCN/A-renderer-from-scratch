#pragma once
#include "v3.h"
#include <algorithm>
#include "ray.h"

inline float ffmin(float a, float b) { return a < b ? a : b; }
inline float ffmax(float a, float b) { return a > b ? a : b; }

class AABB
{
public:
	V3 corners[2];
	AABB(V3 firstPoint);
	AABB(V3 _min, V3 _max) { corners[0] = _min; corners[1] = _max; };
	void AddPoint(V3 p);
	V3 min() const { return corners[0]; }
	V3 max() const { return corners[1]; }

	V3 GetCenter();
	V3 GetDiagnolVector();
	float GetDiagnoalLength();
	int Clip2D(float left, float right, float top, float bottom);
	
	// ray AABB intersect
	bool hit(ray &r, float tmin, float tmax)
	{
		for(int a = 0; a < 3; ++a)
		{
			float invD = 1.0f / r.direction()[a];
			float t0 = (corners[0][a] - r.origin()[a]) * invD;
			float t1 = (corners[1][a] - r.origin()[a]) * invD;
			if (invD < 0.0f)
				std::swap(t0, t1);

			tmin = ffmax(t0, tmin);
			tmax = ffmin(t1, tmax);
			if (tmax <= tmin)
				return false;
		}
		return true;
	}

	friend ostream& operator<<(ostream& ist, AABB bbox);
};
