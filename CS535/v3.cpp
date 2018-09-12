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

void V3::SetColor(unsigned int color)
{
	V3 &v = *this;
	unsigned char*rgba = (unsigned char*)&color;
	v[0] = static_cast<float>(rgba[0]) / 255.0f;
	v[1] = static_cast<float>(rgba[1]) / 255.0f;
	v[2] = static_cast<float>(rgba[2]) / 255.0f;
}

unsigned int V3::GetColor()
{
	// clip to (0.0f,1.0f)
	V3 v = *this;
	unsigned int ret = 0xFF000000;
	unsigned char *rgba = (unsigned char*)&ret;
	for(int i = 0; i < 3; ++i)
	{
		int ichan = static_cast<int>(v[i] * 255.0f);
		ichan = std::max(0, ichan);
		ichan = std::min(ichan, 255);
		rgba[i] = ichan;
	}
	return ret;
}

V3 V3::cross(V3 v1)
{
	V3 v0 = *this;
	return V3(
		v0[1] * v1[2] - v0[2] * v1[1],
		v0[2] * v1[0] - v0[0] * v1[2],
		v0[0] * v1[1] - v0[1] * v1[0]);
}

V3 V3::Rotate(V3 a, float angled)
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
		auto rad = Deg2Rad(angled);
		M33 ao(a, b, c);
		M33 rot;
		rot.SetRotate(0, angled);
		M33 iao = ao.Inverse();
		return iao * rot * ao * v;
	}
	else
	{
		a = a.Normalize();
		b = y.cross(a).Normalize();
		c = a.cross(b).Normalize();
		// from origin to new coord
		auto rad = Deg2Rad(angled);
		M33 ao(b, a, c);
		M33 rot;
		rot.SetRotate(1, angled);
		M33 iao = ao.Inverse();
		return iao * rot * ao * v;
	}
}

V3 V3::RotateThisPointAboutArbitraryAxis(V3 O, V3 a, float angled)
{
	V3 p = *this;
	V3 OP = p - O;
	OP = OP.Rotate(a, angled);
	return OP + O;
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

V3 V3::operator+(V3 v1)
{
	V3 v0 = *this, ret;
	ret[0] = v0[0] + v1[0];
	ret[1] = v0[1] + v1[1];
	ret[2] = v0[2] + v1[2];
	return ret;
}

V3 V3::operator-(V3 v1)
{
	V3 v0 = *this;
	return V3(v0[0] - v1[0], v0[1] - v1[1], v0[2] - v1[2]);
}

V3 V3::operator^(V3 v1)
{
	return cross(v1);
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
