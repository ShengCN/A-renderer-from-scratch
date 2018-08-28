#include "v3.h"

#include "m33.h"

V3::V3(float x)
{
	xyz[0] = xyz[1] = xyz[2] = x;
}

V3::V3(float x, float y, float z) {

	xyz[0] = x;
	xyz[1] = y;
	xyz[2] = z;

}

float& V3::operator[](int i)
{
	return xyz[i];
}

float V3::operator*(V3 v1)
{
	V3 v0 = *this;
	return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];
}

V3 V3::operator-(V3 v1)
{
	V3 v0 = *this;
	return V3(v0[0] - v1[0], v0[1] - v1[1], v0[2] - v1[2]);
}

bool V3::operator==(V3 v1)
{
	auto FloatEqual = [&](float a, float b)
	{
		return std::abs(a - b) < 1e-7;
	};

	V3 v0 = *this;
	return FloatEqual(v0[0], v1[0])
		&& FloatEqual(v0[1], v1[1])
		&& FloatEqual(v0[2], v1[2]);
}

bool V3::operator!=(V3 v1)
{
	V3 v0 = *this;
	return !(v0 == v1);
}

ostream& operator<<(ostream& ost, V3 v)
{
	return ost << v[0] << " " << v[1] << " " << v[2] << endl;
}
