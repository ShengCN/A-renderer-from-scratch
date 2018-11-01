#pragma once
#include <tuple>
#include "v3.h"

// Sphere bounding box
class SBB
{
public:
	V3 c;		// center
	float r;	// radius

	SBB(V3 _c, float _r): c(_c), r(_r) {};

	// whether it is intersect
	// The intersect step t
	tuple<bool, float> RaySBB(V3 rc, V3 ray);
};

