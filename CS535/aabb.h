#pragma once
#include "v3.h"

class AABB
{
public:
	V3 corners[2];
	AABB(V3 firstPoint);
	void AddPoint(V3 p);
	friend ostream& operator<<(ostream& ist, AABB bbox);
};

