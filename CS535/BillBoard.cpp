#include "BillBoard.h"
#include "MathTool.h"
#include <fstream>

BillBoard::BillBoard()
{
}

BillBoard::~BillBoard()
{
}

void BillBoard::SetBillboard(V3 O, V3 n, V3 up, float sz, float s, float t)
{
	mesh = make_shared<TM>();
	mesh->SetBillboard(O, n, up, sz, s, t);
}

void BillBoard::GPUsetBBtexture()
{
	texID = fbTexture->SaveCPU2GPUtexture();
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

M33 BillBoard::GetCorners()
{
	M33 corners;
	corners[0] = mesh->verts[0];
	corners[1] = mesh->verts[1];
	corners[2] = mesh->verts[3];
	return corners;
}

V3 BillBoard::GetColor(V3 p, float& alpha)
{
	if (!fbTexture || !InsideBillboard(p))
	{
		alpha = 0.0f;
		return V3(0.0f);
	}

	float s, t;
	GetST(p, s, t);

	return fbTexture->BilinearLookupColor(s, t, alpha);
}

V3 BillBoard::GetColor(FrameBuffer *fb, V3 p)
{
	if (!InsideBillboard(p))
		return V3(0.0f);

	float s, t;
	GetST(p, s, t);

	auto &tex = fb->textures.at(mesh->tex).back();
	return fb->BilinearLookupColor(tex, s, t);
}

V3 BillBoard::GetColor(FrameBuffer* fb, V3 p, float& alpha)
{
	if (!InsideBillboard(p))
	{
		alpha = 0.0f;
		return V3(0.0f);
	}
	float s, t;
	GetST(p, s, t);

	return  fb->LookupColor(mesh->tex, s, t, alpha);
}

void BillBoard::RenderBB(PPC* ppc, FrameBuffer* fb)
{
	mesh->RenderBB(ppc, fb, fbTexture.get());
}

void BillBoard::GetST(V3 p, float& s, float& t)
{
	V3 p0 = mesh->verts[0];
	V3 p1 = mesh->verts[1];
	V3 p3 = mesh->verts[3];
	V3 x = p3 - p0;
	V3 y = p1 - p0;
	V3 v = p - p0;
	float xf = x * v, yf = y * v;
	s = xf / (x.Length() * x.Length());
	t = yf / (y.Length() * y.Length());
}
