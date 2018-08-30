#include "v3.h"

#include "m33.h"
#include "MathTool.h"

V3::V3(float x)
{
	xyz[0] = xyz[1] = xyz[2] = x;
}

V3::V3(float x, float y, float z) {

	xyz[0] = x;
	xyz[1] = y;
	xyz[2] = z;

}

V3 V3::Normalize()
{
	V3 v = *this;
	return v / Length();
}

float V3::Length()
{
	V3 v = *this;
	return std::sqrt(v*v);
}

V3 V3::cross(V3 v1)
{
	V3 v0 = *this;
	return V3(
		v0[1] * v1[2] - v0[2] * v1[1],
		v0[2] * v1[0] - v0[0] * v1[2],
		v0[0] * v1[1] - v0[1] * v1[0]);
}

V3 V3::Rotate(V3 a, float angle)
{
	V3 v = *this, x(1.0f,0.0f,0.0f),y(0.0f,1.0f,0.0f),b,c, ret;
	auto xa = abs(x*a);
	auto ya = abs(y*a);
	if(xa<ya)
	{
		a = a.Normalize();
		b = x.cross(a).Normalize();
		c = a.cross(b).Normalize();
		// from origin to new coord
		auto rad = Deg2Rad(angle);
		M33 ao(a, b, c);
		M33 rot;
		rot.SetRotate(0, angle);
		M33 iao = ao.Inverse();
		return iao * rot * ao * v;
	}
	else
	{
		a = a.Normalize();
		b = y.cross(a).Normalize();
		c = a.cross(b).Normalize();
		// from origin to new coord
		auto rad = Deg2Rad(angle);
		M33 ao(b, a, c);
		M33 rot;
		rot.SetRotate(1, angle);
		M33 iao = ao.Inverse();
		return iao * rot * ao * v;
	}
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

V3 V3::operator*(float scf)
{
	V3 v = *this;
	return V3(v[0] * scf, v[1] * scf, v[2] * scf);
}

V3 V3::operator/(float scf)
{
	V3 v = *this;
	return V3(v[0] / scf, v[1] / scf, v[2] / scf);
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
		return std::abs(a - b) < 1e-6;
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

istream& operator>>(istream& ist, V3 &v)
{
	return ist >> v[0] >> v[1] >> v[2];
}

ostream& operator<<(ostream& ost, V3 v)
{
	return ost << v[0] << " " << v[1] << " " << v[2] << endl;
}
