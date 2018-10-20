#pragma once
#include "v3.h"

class AABB
{
public:
	V3 corners[2];
	AABB(V3 firstPoint);
	void AddPoint(V3 p);
	V3 GetCenter();
	V3 GetDiagnolVector();
	float GetDiagnoalLength();
	template<typename T> int Clip2D(T left, T right, T top, T bottom);

	friend ostream& operator<<(ostream& ist, AABB bbox);
};


template<typename T>
inline int AABB::Clip2D(T left, T right, T top, T bottom)
{
	V3 c0 = corners[0];
	V3 c1 = corners[1];
	if (c0[0] > right || c1[0] < left || c0[1] > bottom || c1[1] < top)
		return 0;
	if (c0[0] < left)
		c0[0] = left;
	if (c1[0] > right)
		c1[0] = right;
	if (c0[1] < top)
		c0[1] = top;
	if (c1[1] > bottom)
		c1[1] = bottom;
	return 1;
}
