#include <fstream>
#include "PPC.h"
#include "MathTool.h"
#include "m33.h"


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

// input pp is the center of the pixel
V3 PPC::Unproject(V3 pp)
{
	return C + (a*pp[0] + b * pp[1] + c) * (1.0f / pp[2]);
}

V3 PPC::UnprojectPixel(float uf, float vf, float currf)
{
	return C + (a*uf + b * vf + c)*currf * (1.0f / GetFocal());
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
	V3 rC = C.RotateThisPointAboutArbitraryAxis(p, V3(0.0f, 1.0f, 0.0f), theta);
	
	PositionAndOrient(rC, p, V3(0.0f, 1.0f, 0.0f));
}

void PPC::RevolveV(V3 p, float theta)
{
	V3 rC = C.RotateThisPointAboutArbitraryAxis(p, V3(1.0f, 0.0f, 0.0f), theta);

	PositionAndOrient(rC, p, V3(0.0f, 1.0f, 0.0f));
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

void PPC::SetInterpolated(PPC* ppc0, PPC* ppc1, float fract)
{
	V3 ppcV = ppc1->C - ppc0->C;
	V3 ppc0vd = (ppc0->a ^ ppc0->b).UnitVector();
	V3 ppc1vd = (ppc1->a ^ ppc1->b).UnitVector();
	V3 vd = ppc0vd + (ppc1vd - ppc0vd)*fract;
	float f = ppc0->GetFocal() + (ppc1->GetFocal() - ppc0->GetFocal())*fract;

	C = ppc0->C + ppcV * fract;
	a = ppc0->a + (ppc1->a - ppc0->a)*fract;
	b = (vd^a).UnitVector();
	c = vd * f - a * static_cast<float>(w) / 2.0f - b * static_cast<float>(h) / 2.0f;
}

void PPC::SaveBin(std::string fname)
{
	ofstream of(fname.c_str(), ofstream::out | ostream::binary);
	if (of.is_open())
	{
		of.write((char *)&C, sizeof(float) * 3);
		of.write((char *)&a, sizeof(float) * 3);
		of.write((char *)&b, sizeof(float) * 3);
		of.write((char *)&c, sizeof(float) * 3);
	}
	else
		cerr << "File " << fname.c_str() << " cannot open to write!\n";
	
	cerr << "File " << fname.c_str() << " written!\n";
	of.close();
}

void PPC::LoadBin(std::string fname)
{
	ifstream input(fname.c_str(),ifstream::in | ifstream::binary);
	if (input.is_open())
	{
		input.read((char*)&C, sizeof(float) * 3);
		input.read((char*)&a, sizeof(float) * 3);
		input.read((char*)&b, sizeof(float) * 3);
		input.read((char*)&c, sizeof(float) * 3);
	}
	else
		cerr << "File " << fname.c_str() << " cannot open!\n";
	
	input.close();
}

