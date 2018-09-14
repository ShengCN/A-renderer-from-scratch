#include "AABB.h"
#include <algorithm>

AABB::AABB(V3 firstPoint)
{
	corners[0] = firstPoint;
	corners[1] = firstPoint;
}

void AABB::AddPoint(V3 p)
{
	for(int i = 0; i <3; ++i)
	{
		corners[0][i] = std::min(p[i], corners[0][i]);
		corners[1][i] = std::max(p[i], corners[1][i]);
	}
}

V3 AABB::GetCenter()
{
	return (corners[0] + corners[1]) * 0.5f;
}

float AABB::GetDiagnoalLength()
{
	V3 diagonalV = corners[1] - corners[0];
	return diagonalV.Length();
}

ostream& operator<<(ostream& ist, AABB bbox)
{
	return ist << "BBox corners: " << bbox.corners[0] << bbox.corners[1];
}
