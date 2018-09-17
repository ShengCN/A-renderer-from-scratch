#include "PPC.h"
#include "MathTool.h"
#include "m33.h"
#include <valarray>


PPC::PPC(int _w, int _h, float hfov):w(_w), h(_h)
{
	C = V3(0.0f);
	a = V3(1.0f, 0.0f, 0.0f);
	b = V3(0.0f, -1.0f, 0.0f);
	c = V3(-0.5f*w, 0.5f*h, -0.5f*w / tan(Deg2Rad(hfov/2.0f)));
}

void PPC::Translate(V3 v)
{
	C = C + v;
}

int PPC::Project(V3 P, V3& ProjP)
{
	M33 proj;
	proj.SetColumn(0, a);
	proj.SetColumn(1, b);
	proj.SetColumn(2, c);
	V3 PPixel = proj.Inverse()*(P - C);

	// check if in the view frustrum
	if (w <= 0.0f)
		return 0;

	auto w = PPixel[2];
	auto u = PPixel[0] / w;
	auto v = PPixel[1] / w;
	ProjP = V3(u, v, 1.0f / w);
	return 1;
}

V3 PPC::GetVD()
{
	return (a^b).UnitVector();
}

float PPC::GetFocal()
{
	return (a^b).UnitVector() * c;
}

float PPC::GetHorizontalFOV()
{
	return 2.0f * Rad2Deg(atan(static_cast<float>(w) / 2.0f * a.Length() / GetFocal()));
}

float PPC::GetVerticalFOV()
{
	return 2.0f * Rad2Deg(atan(static_cast<float>(h) / 2.0f * b.Length() / GetFocal()));
}

V3 PPC::GetRay(int u, int v)
{
	return c + a * (static_cast<float>(u) + 0.5f) + b * (static_cast<float>(v) + 0.5f);
}

V3 PPC::GetRayCenter(int u, int v)
{
	return C + GetRay(u, v);
}

void PPC::Pan(float theta)
{
	a = a.Rotate(b.UnitVector()*(-1.0f), theta);
	c = c.Rotate(b.UnitVector()*(-1.0f), theta);
}

void PPC::Tilt(float theta)
{
	b = b.Rotate(a.UnitVector(), theta);
	c = c.Rotate(c.UnitVector(), theta);
}

void PPC::Roll(float theta)
{
	V3 dir = (a ^ b).UnitVector();
	a = a.Rotate(dir, theta);
	b = b.Rotate(dir, theta);
	c = c.Rotate(dir, theta);
}

void PPC::RevolveH(V3 p, float theta)
{
	// float f = GetFocal();
	V3 rC = C.RotateThisPointAboutArbitraryAxis(p, V3(0.0f, 1.0f, 0.0f), theta);
	// V3 vd = (p - rC).UnitVector();
 //
	// V3 ra = (vd ^ V3(0.0f, 1.0f, 0.0f)).UnitVector() * a.Length();
	// V3 rb = (vd ^ ra).UnitVector() * b.Length();
	// V3 rc = vd * f - ra * static_cast<float>(w) / 2.0f - rb * static_cast<float>(h) / 2.0f;
 //
	// // update ppc intrinsic parameters
	// C = rC;
	// a = ra;
	// b = rb;
	// c = rc;

	PositionAndOrient(rC, p, V3(0.0f, 1.0f, 0.0f));
}

void PPC::RevolveV(V3 p, float theta)
{
	// view direction
	float focal = GetFocal();

	V3 vd = (C - p).UnitVector();
	vd = vd.Rotate(V3(1.0f, 0.0f, 0.0f), theta);

	b = vd ^ V3(1.0f, 0.0f, 0.0f);
	a = vd ^ (b * -1.0f);
	// c + w / 2 * a + h / 2 * b = vd
	c = vd - a * static_cast<float>(w) / 2.0f - b * static_cast<float>(h) / 2.0f;
	C = p + vd * focal;
}

void PPC::PositionAndOrient(V3 newC, V3 lap, V3 up)
{
	V3 newa, newb, newc;
	V3 newvd = (lap - newC).UnitVector();
	float f = GetFocal();
	newa = (newvd ^ up).UnitVector() * a.Length();
	newb = (newvd ^ newa).UnitVector() * b.Length();
	newc = (newvd * f) - newa * static_cast<float>(w) / 2.0f - newb * static_cast<float>(h) / 2.0f;

	a = newa;
	b = newb;
	c = newc;
	C = newC;
}

void PPC::Zoom(float scf)
{
	V3 vd = GetVD();
	float f = GetFocal();
	float newf = f * scf;
	c = c + vd * (newf-f);
}

