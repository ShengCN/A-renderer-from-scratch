#include "SBB.h"

tuple<bool, float> SBB::RaySBB(V3 rc, V3 ray)
{
	bool isIntersect = false; float t = 0.0f;

	// |ray * t + rc-c| < r ?
	// b^2 - 4ac
	float a = ray * ray, b = ray * (rc - c) * 2.0f, c1 = (rc - c) * (rc - c) - r * r;
	float delta = b * b - a * c1 * 4.0f;
	
	// have intersection, update isIntersect, t
	if (delta >= 0)
	{
		isIntersect = true;
		t = (-b - a * c1 * 4.0f) / (a * 2.0f);
	}

	return tuple<bool, float>(isIntersect, t);
}
