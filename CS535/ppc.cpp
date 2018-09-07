#include "PPC.h"
#include "MathTool.h"
#include "m33.h"


PPC::PPC(int _w, int _h, float hfov):w(_w), h(_h)
{
	C = V3(0.0f);
	a = V3(1.0f, 0.0f, 0.0f);
	b = V3(0.0f, -1.0f, 0.0f);
	c = V3(-0.5f*w, 0.5f*h, -0.5f*w / tan(Deg2Rad(hfov)));
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
	auto w = PPixel[2];
	auto u = PPixel[0] / w;
	auto v = PPixel[1] / w;

	// check if in the view frustrum
	if (w <= 0.0f)
		return 0;
	
	ProjP = V3(u, v, w);
	return 1;
}
