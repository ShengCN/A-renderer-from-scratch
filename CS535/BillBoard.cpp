#include "BillBoard.h"
#include "MathTool.h"
#include <fstream>

BillBoard::BillBoard()
{
}


void BillBoard::SetBillboard(V3 O, V3 n, V3 up, float sz)
{
	mesh = make_shared<TM>();
	mesh->SetBillboard(O, n, up, sz);
}

bool BillBoard::Intersect(V3 p, V3 d, float &t)
{
	V3 b0 = mesh->verts[0];
	V3 bn = (mesh->normals[0]).UnitVector();
	t = (b0 - p) * bn / (d * bn);
	if (t < 0.0f)
		return false;

	V3 pb = p + d * t; // point in bb
	return InsideBillboard(pb);
}

bool BillBoard::InsideBillboard(V3 p)
{
	V3 p0 = mesh->verts[0];
	V3 p1 = mesh->verts[1];
	V3 p3 = mesh->verts[3];
	V3 n = mesh->normals[0];

	if(!FloatEqual((p-p0) * n,0.0f))
		return false;

	V3 x = p3 - p0;
	V3 y = p1 - p0;
	V3 v = p - p0;
	float xf = x * v, yf = y * v;
	return !(xf < 0.0f || yf < 0.0f || xf / x.Length() > x.Length() || yf / y.Length() > y.Length());
}

V3 BillBoard::GetColor(FrameBuffer *fb, V3 p)
{
	if (!InsideBillboard(p))
		return V3(0.0f);

	V3 p0 = mesh->verts[0];
	V3 p1 = mesh->verts[1];
	V3 p3 = mesh->verts[3];
	V3 x = p3 - p0;
	V3 y = p1 - p0;
	V3 v = p - p0;
	float xf = x * v, yf = y * v;
	float s = xf / (x.Length() * x.Length());
	float t = yf / (y.Length() * y.Length());

	auto &tex = fb->textures.at(mesh->tex).back();
	return fb->BilinearLookupColor(tex, s, t);
}

BillBoard::~BillBoard()
{
}
