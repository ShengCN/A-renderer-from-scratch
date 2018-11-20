#include <fstream>
#include <algorithm>
#include <complex>
#include <omp.h>

#include "TM.h"
#include "MathTool.h"
#include "GlobalVariables.h"
#include "m33.h"

using namespace std;

void TM::SetRectangle(V3 O, float rw, float rh)
{
	vertsN = 4;
	trisN = 2;
	Allocate();

	verts[0] = O + V3(-rw / 2.0f, rh / 2.0f, 0.0f);
	verts[1] = O + V3(-rw / 2.0f, -rh / 2.0f, 0.0f);
	verts[2] = O + V3(rw / 2.0f, -rh / 2.0f, 0.0f);
	verts[3] = O + V3(rw / 2.0f, rh / 2.0f, 0.0f);

	// First Triangle
	int tri = 0;
	tris[3 * tri + 0] = 0;
	tris[3 * tri + 1] = 1;
	tris[3 * tri + 2] = 2;
	++tri;

	// Second Triangle
	tris[3 * tri + 0] = 2;
	tris[3 * tri + 1] = 3;
	tris[3 * tri + 2] = 0;

	for (int vi = 0; vi < vertsN; ++vi)
	{
		colors[vi] = V3(0.0f);
	}
}

void TM::SetTriangle(PointProperty p0, PointProperty p1, PointProperty p2)
{
	vertsN = 3;
	trisN = 1;
	Allocate();

	verts[0] = p0.p;
	verts[1] = p1.p;
	verts[2] = p2.p;

	colors[0] = p0.c;
	colors[1] = p1.c;
	colors[2] = p2.c;

	tris[0] = 0;
	tris[1] = 1;
	tris[2] = 2;
}

void TM::SetQuad(PointProperty p0, PointProperty p1, PointProperty p2, PointProperty p3)
{
	vertsN = 4;
	trisN = 2;
	Allocate();

	verts[0] = p0.p;
	verts[1] = p1.p;
	verts[2] = p2.p;
	verts[3] = p3.p;

	colors[0] = p0.c;
	colors[1] = p1.c;
	colors[2] = p2.c;
	colors[3] = p3.c;

	normals[0] = p0.n;
	normals[1] = p1.n;
	normals[2] = p2.n;
	normals[3] = p3.n;

	vertST[0] = p0.s;
	vertST[1] = p0.t;
	vertST[2] = p1.s;
	vertST[3] = p1.t;
	vertST[4] = p2.s;
	vertST[5] = p2.t;
	vertST[6] = p3.s;
	vertST[7] = p3.t;

	tris[0] = 0;
	tris[1] = 1;
	tris[2] = 2;

	tris[3] = 2;
	tris[4] = 3;
	tris[5] = 0;
}

void TM::SetQuad(V3 O, V3 n, V3 up, float sz, float s, float t)
{
	up = up.UnitVector();
	V3 right = (up ^ n).UnitVector();
	V3 p0 = O + up * sz - right * sz;
	V3 p1 = O - up * sz - right * sz;
	V3 p2 = O - up * sz + right * sz;
	V3 p3 = O + up * sz + right * sz;
	V3 c0(0.0f), c1(0.0f), c2(0.0f), c3(0.0f);
	PointProperty pp0(p0, c0, n, 0.0f, 0.0f);
	PointProperty pp1(p1, c1, n, 0.0f, t);
	PointProperty pp2(p2, c2, n, s, t);
	PointProperty pp3(p3, c3, n, s, 0.0f);
	SetQuad(pp0, pp1, pp2, pp3);
}

void TM::SetBillboard(V3 O, V3 n, V3 up, float sz, float s, float t)
{
	up = up.UnitVector();
	V3 right = (up ^ n).UnitVector();
	V3 p0 = O + up * sz - right * sz;
	V3 p1 = O - up * sz - right * sz;
	V3 p2 = O - up * sz + right * sz;
	V3 p3 = O + up * sz + right * sz;
	V3 c0(0.0f), c1(0.0f), c2(0.0f), c3(0.0f);
	PointProperty pp0(p0, c0, n, 0.0f, 0.0f);
	PointProperty pp1(p1, c1, n, 0.0f, t);
	PointProperty pp2(p2, c2, n, s, t);
	PointProperty pp3(p3, c3, n, s, 0.0f);
	isEnvMapping = false;
	SetQuad(pp0, pp1, pp2, pp3);
}

void TM::SetText(string tf)
{
	tex = tf;
}

void TM::Allocate()
{
	verts.resize(vertsN);
	colors.resize(vertsN);
	normals.resize(vertsN);
	vertST.resize(2 * vertsN);
	tris.resize(3 * trisN); // each triangle has three topological indexs
}

void TM::RenderPoints(PPC* ppc, FrameBuffer* fb)
{
	for (int vi = 0; vi < vertsN; ++vi)
	{
		fb->Draw3DPoint(ppc, verts[vi], 0xFF000000, 7);
	}
}

void TM::RenderWireFrame(PPC* ppc, FrameBuffer* fb)
{
	for (int ti = 0; ti < trisN; ++ti)
	{
		// For each triangle, draw three line segments
		for (int ei = 0; ei < 3; ++ei)
		{
			int vi0 = tris[ti * 3 + ei];
			int vi1 = tris[ti * 3 + (ei + 1) % 3];
			fb->Draw3DSegment(ppc, verts[vi0], colors[vi0], verts[vi1], colors[vi1]);
		}
	}
}

void TM::RenderFill(PPC* ppc, FrameBuffer* fb)
{
	auto gv = GlobalVariables::Instance();

	if (gv->lodTexture)
	{
		// Lod Level, use bbox to estimate how many pixels do we need
		// Assume square texture
		auto aabb = ComputeAABB();
		V3 paabb0(0.0f), paabb1(0.0f);
		ppc->Project(aabb.corners[0], paabb0);
		ppc->Project(aabb.corners[1], paabb1);
		V3 paabbV = paabb1 - paabb0;
		pixelSz = max(abs(paabbV[0]), abs(paabbV[1]));
		pixelSz = Clamp(pixelSz, 0, fb->w);
		// cerr << "Current LoD: " << log2(pixelSz) << endl;
	}
	else
		pixelSz = -1;

	// with texture
	for (int ti = 0; ti < trisN; ++ti)
	{
		int vi0 = tris[ti * 3 + 0];
		int vi1 = tris[ti * 3 + 1];
		int vi2 = tris[ti * 3 + 2];

		bool hasTexture = false;
		if (vertST.size() == verts.size() * 2) hasTexture = true;

		PointProperty p0(verts[vi0], colors[vi0], normals[vi0], hasTexture ? vertST[vi0 * 2] : 0.0f,
		                 hasTexture ? vertST[vi0 * 2 + 1] : 0.0f);
		PointProperty p1(verts[vi1], colors[vi1], normals[vi1], hasTexture ? vertST[vi1 * 2] : 0.0f,
		                 hasTexture ? vertST[vi1 * 2 + 1] : 0.0f);
		PointProperty p2(verts[vi2], colors[vi2], normals[vi2], hasTexture ? vertST[vi2 * 2] : 0.0f,
		                 hasTexture ? vertST[vi2 * 2 + 1] : 0.0f);

		// According to loD, do trilinear in texture look up
		V3 pp0, pp1, pp2;
		if (!ppc->Project(p0.p, pp0))
			return;
		if (!ppc->Project(p1.p, pp1))
			return;
		if (!ppc->Project(p2.p, pp2))
			return;

		if (pp0[0] == FLT_MAX ||
			pp1[0] == FLT_MAX ||
			pp2[0] == FLT_MAX)
			return;

		AABB bbTri(pp0);
		bbTri.AddPoint(pp1);
		bbTri.AddPoint(pp2);
		if (!bbTri.Clip2D(0, fb->w - 1, 0, fb->h - 1))
			return;

		M33 abcM;
		abcM.SetColumn(0, ppc->a);
		abcM.SetColumn(1, ppc->b);
		abcM.SetColumn(2, ppc->c);
		M33 vcM;
		vcM.SetColumn(0, p0.p - ppc->C);
		vcM.SetColumn(1, p1.p - ppc->C);
		vcM.SetColumn(2, p2.p - ppc->C);
		M33 qM = vcM.Inverse() * abcM;

		// Rasterize bbox
		int left = static_cast<int>(bbTri.corners[0][0] + 0.5f), right = static_cast<int>(bbTri.corners[1][0] - 0.5f);
		int top = static_cast<int>(bbTri.corners[0][1] + 0.5f), bottom = static_cast<int>(bbTri.corners[1][1] - 0.5f);

		for (int v = top; v <= bottom; ++v)
		{
			for (int u = left; u <= right; ++u)
			{
				V3 uvP(static_cast<float>(u) + 0.5f, static_cast<float>(v) + 0.5f, 1.0f);
				bool s1 = Side2D(uvP, pp0, pp1, pp2);
				bool s2 = Side2D(uvP, pp1, pp2, pp0);
				bool s3 = Side2D(uvP, pp2, pp0, pp1);

				if (s1 && s2 && s3)
				{
					float div = (qM.GetColumn(0) * V3(u, u, u) + qM.GetColumn(1) * V3(v, v, v) + qM.GetColumn(2) *
						V3(1.0f));

					if (FloatEqual(div, 0.0f))
						continue;

					div = 1.0f / div;
					float wv = qM.GetColumn(0) * V3(u, u, u) + qM.GetColumn(1) * V3(v, v, v) + qM.GetColumn(2) *
						V3(1.0f);

					if (gv->depthTest && !fb->DepthTest(u, v, wv))
						continue;

					uvP[2] = wv;
					float k = V3(u, v, 1.0f) * qM[1] * div;
					float l = V3(u, v, 1.0f) * qM[2] * div;
					V3 st0(p0.s, p0.t, 0.0f), st1(p1.s, p1.t, 0.0f), st2(p2.s, p2.t, 0.0f);
					V3 st = st0 + (st1 - st0) * k + (st2 - st0) * l;
					V3 pc = p0.c + (p1.c - p0.c) * k + (p2.c - p0.c) * l;
					V3 pn = p0.n + (p1.n - p0.n) * k + (p2.n - p0.n) * l;
					V3 p = ppc->Unproject(uvP);

					// normal derivative (approximately)
					auto GetN = [&](int u, int v)
					{
						float k = V3(u, v, 1.0f) * qM[1] * div;
						float l = V3(u, v, 1.0f) * qM[2] * div;
						return p0.c + (p1.c - p0.c) * k + (p2.c - p0.c) * l;
					};
					V3 du = (GetN(u + 1, v) - GetN(u - 1, v)) * 0.5f;
					V3 dv = (GetN(u, v + 1) - GetN(u, v - 1)) * 0.5f;
					V3 dudv = du + dv;

					// shading
					PointProperty pp(p, pc, pn, st[0], st[1]);

					auto [color , alpha] = gv->isCubemapMipmap ? Shading(ppc, fb, u, v, wv, pp, dudv) : Shading(ppc, fb, u, v, wv, pp);

					// alpha blending 
					if (!FloatEqual(alpha, 1.0f))
					{
						V3 bgC(0.0f);
						bgC.SetColor(fb->Get(u, v));
						color = color * alpha + bgC * (1.0f - alpha);
					}

					fb->DrawPoint(u, v, color.GetColor());
				}
			}
		}
	}
}

void TM::RenderFillZ(PPC* ppc, FrameBuffer* fb)
{
	auto gv = GlobalVariables::Instance();
	// Lod Level, use bbox to estimate how many pixels do we need
	// Assume square texture
	auto aabb = ComputeAABB();
	V3 paabb0(0.0f), paabb1(0.0f);
	ppc->Project(aabb.corners[0], paabb0);
	ppc->Project(aabb.corners[1], paabb1);
	V3 paabbV = paabb1 - paabb0;
	pixelSz = max(abs(paabbV[0]), abs(paabbV[1]));
	pixelSz = Clamp(pixelSz, 0, fb->w);
	// cerr << "Current LoD: " << log2(pixelSz) << endl;

	// with texture
	for (int ti = 0; ti < trisN; ++ti)
	{
		int vi0 = tris[ti * 3 + 0];
		int vi1 = tris[ti * 3 + 1];
		int vi2 = tris[ti * 3 + 2];

		bool hasTexture = false;
		if (vertST.size() == verts.size() * 2) hasTexture = true;

		PointProperty p0(verts[vi0], colors[vi0], normals[vi0], hasTexture ? vertST[vi0 * 2] : 0.0f,
		                 hasTexture ? vertST[vi0 * 2 + 1] : 0.0f);
		PointProperty p1(verts[vi1], colors[vi1], normals[vi1], hasTexture ? vertST[vi1 * 2] : 0.0f,
		                 hasTexture ? vertST[vi1 * 2 + 1] : 0.0f);
		PointProperty p2(verts[vi2], colors[vi2], normals[vi2], hasTexture ? vertST[vi2 * 2] : 0.0f,
		                 hasTexture ? vertST[vi2 * 2 + 1] : 0.0f);

		// According to loD, do trilinear in texture look up
		V3 pp0, pp1, pp2;
		if (!ppc->Project(p0.p, pp0))
			return;
		if (!ppc->Project(p1.p, pp1))
			return;
		if (!ppc->Project(p2.p, pp2))
			return;

		if (pp0[0] == FLT_MAX ||
			pp1[0] == FLT_MAX ||
			pp2[0] == FLT_MAX)
			return;

		AABB bbTri(pp0);
		bbTri.AddPoint(pp1);
		bbTri.AddPoint(pp2);
		if (!bbTri.Clip2D(0, fb->w - 1, 0, fb->h - 1))
			return;

		M33 abcM;
		abcM.SetColumn(0, ppc->a);
		abcM.SetColumn(1, ppc->b);
		abcM.SetColumn(2, ppc->c);
		M33 vcM;
		vcM.SetColumn(0, p0.p - ppc->C);
		vcM.SetColumn(1, p1.p - ppc->C);
		vcM.SetColumn(2, p2.p - ppc->C);
		M33 qM = vcM.Inverse() * abcM;

		// Rasterize bbox
		int left = static_cast<int>(bbTri.corners[0][0] + 0.5f), right = static_cast<int>(bbTri.corners[1][0] - 0.5f);
		int top = static_cast<int>(bbTri.corners[0][1] + 0.5f), bottom = static_cast<int>(bbTri.corners[1][1] - 0.5f);

		for (int v = top; v <= bottom; ++v)
		{
			for (int u = left; u <= right; ++u)
			{
				V3 uvP(static_cast<float>(u) + 0.5f, static_cast<float>(v) + 0.5f, 1.0f);
				bool s1 = Side2D(uvP, pp0, pp1, pp2);
				bool s2 = Side2D(uvP, pp1, pp2, pp0);
				bool s3 = Side2D(uvP, pp2, pp0, pp1);

				if (s1 && s2 && s3)
				{
					float wv = qM.GetColumn(0) * V3(u, u, u) + qM.GetColumn(1) * V3(v, v, v) + qM.GetColumn(2) *
						V3(1.0f);
					if (gv->depthTest && !fb->DepthTest(u, v, wv))
						continue;

					if (gv->isDebugZbuffer)
						fb->DrawPoint(u, v, V3(1.0f / wv).GetColor());
				}
			}
		}
	}
}

void TM::RenderAABB(PPC* ppc, FrameBuffer* fb)
{
	auto aabb = ComputeAABB();
	V3 x(1.0f, 0.0f, 0.0f), y(0.0f, 1.0f, 0.0f), z(0.0f, 0.0f, 1.0f);
	V3 lc = aabb.corners[0], rc = aabb.corners[1]; // left corner and right corner, left "smaller" than right
	float dx = (rc - lc) * x;
	float dy = (rc - lc) * y;
	float dz = (rc - lc) * z;

	V3 p0, p1, p2, p3;
	V3 p4, p5, p6, p7;
	p0 = lc;
	p1 = lc + x * dx;
	p2 = lc + y * dy;
	p3 = lc + x * dx + y * dy;

	p4 = p0 + z * dz;
	p5 = p1 + z * dz;
	p6 = p2 + z * dz;
	p7 = p3 + z * dz;

	V3 blue(0.0f, 0.0f, 1.0f);
	fb->Draw3DSegment(ppc, p0, blue, p1, blue);
	fb->Draw3DSegment(ppc, p0, blue, p2, blue);
	fb->Draw3DSegment(ppc, p1, blue, p3, blue);
	fb->Draw3DSegment(ppc, p2, blue, p3, blue);

	fb->Draw3DSegment(ppc, p0, blue, p4, blue);
	fb->Draw3DSegment(ppc, p1, blue, p5, blue);
	fb->Draw3DSegment(ppc, p2, blue, p6, blue);
	fb->Draw3DSegment(ppc, p3, blue, p7, blue);

	fb->Draw3DSegment(ppc, p4, blue, p5, blue);
	fb->Draw3DSegment(ppc, p4, blue, p6, blue);
	fb->Draw3DSegment(ppc, p5, blue, p7, blue);
	fb->Draw3DSegment(ppc, p6, blue, p7, blue);
}

void TM::RenderBB(PPC* ppc, FrameBuffer* fb, FrameBuffer* bbTexture)
{
	auto gv = GlobalVariables::Instance();
	for (int ti = 0; ti < trisN; ++ti)
	{
		int vi0 = tris[ti * 3 + 0];
		int vi1 = tris[ti * 3 + 1];
		int vi2 = tris[ti * 3 + 2];

		bool hasTexture = false;
		if (vertST.size() == verts.size() * 2) hasTexture = true;

		PointProperty p0(verts[vi0], colors[vi0], normals[vi0], hasTexture ? vertST[vi0 * 2] : 0.0f,
		                 hasTexture ? vertST[vi0 * 2 + 1] : 0.0f);
		PointProperty p1(verts[vi1], colors[vi1], normals[vi1], hasTexture ? vertST[vi1 * 2] : 0.0f,
		                 hasTexture ? vertST[vi1 * 2 + 1] : 0.0f);
		PointProperty p2(verts[vi2], colors[vi2], normals[vi2], hasTexture ? vertST[vi2 * 2] : 0.0f,
		                 hasTexture ? vertST[vi2 * 2 + 1] : 0.0f);

		// According to loD, do trilinear in texture look up
		V3 pp0, pp1, pp2;
		if (!ppc->Project(p0.p, pp0))
			return;
		if (!ppc->Project(p1.p, pp1))
			return;
		if (!ppc->Project(p2.p, pp2))
			return;

		if (pp0[0] == FLT_MAX ||
			pp1[0] == FLT_MAX ||
			pp2[0] == FLT_MAX)
			return;

		AABB bbTri(pp0);
		bbTri.AddPoint(pp1);
		bbTri.AddPoint(pp2);
		if (!bbTri.Clip2D(0, fb->w - 1, 0, fb->h - 1))
			return;

		M33 abcM;
		abcM.SetColumn(0, ppc->a);
		abcM.SetColumn(1, ppc->b);
		abcM.SetColumn(2, ppc->c);
		M33 vcM;
		vcM.SetColumn(0, p0.p - ppc->C);
		vcM.SetColumn(1, p1.p - ppc->C);
		vcM.SetColumn(2, p2.p - ppc->C);
		M33 qM = vcM.Inverse() * abcM;

		// Rasterize bbox
		int left = static_cast<int>(bbTri.corners[0][0] + 0.5f), right = static_cast<int>(bbTri.corners[1][0] - 0.5f);
		int top = static_cast<int>(bbTri.corners[0][1] + 0.5f), bottom = static_cast<int>(bbTri.corners[1][1] - 0.5f);

		for (int v = top; v <= bottom; ++v)
		{
			for (int u = left; u <= right; ++u)
			{
				V3 uvP(static_cast<float>(u) + 0.5f, static_cast<float>(v) + 0.5f, 1.0f);
				bool s1 = Side2D(uvP, pp0, pp1, pp2);
				bool s2 = Side2D(uvP, pp1, pp2, pp0);
				bool s3 = Side2D(uvP, pp2, pp0, pp1);

				if (s1 && s2 && s3)
				{
					float div = (qM.GetColumn(0) * V3(u, u, u) + qM.GetColumn(1) * V3(v, v, v) + qM.GetColumn(2) *
						V3(1.0f));

					if (FloatEqual(div, 0.0f) || isnan(div))
						continue;

					div = 1.0f / div;
					float wv = qM.GetColumn(0) * V3(u, u, u) + qM.GetColumn(1) * V3(v, v, v) + qM.GetColumn(2) *
						V3(1.0f);

					if (gv->depthTest && !fb->DepthTest(u, v, wv))
						continue;

					uvP[2] = wv;
					float k = V3(u, v, 1.0f) * qM[1] * div;
					float l = V3(u, v, 1.0f) * qM[2] * div;
					V3 st0(p0.s, p0.t, 0.0f), st1(p1.s, p1.t, 0.0f), st2(p2.s, p2.t, 0.0f);
					V3 st = st0 + (st1 - st0) * k + (st2 - st0) * l;

					// shading
					float s = st[0], t = st[1], alpha = 0.0f;
					V3 color = bbTexture->BilinearLookupColor(s, t, alpha);

					// alpha blending 
					if (!FloatEqual(alpha, 1.0f))
					{
						V3 bgC(0.0f);
						bgC.SetColor(fb->Get(u, v));
						color = color * alpha + bgC * (1.0f - alpha);
					}

					fb->DrawPoint(u, v, color.GetColor());
				}
			}
		}
	}
}

void TM::RenderHW()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, &verts[0]);
	glColorPointer(3, GL_FLOAT, 0, &colors[0]);
	glNormalPointer(GL_FLOAT, 0, &normals[0]);

	glDrawElements(GL_TRIANGLES, 3 * trisN, GL_UNSIGNED_INT, &tris[0]);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

void TM::RenderHWWireframe()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, &verts[0]);
	glColorPointer(3, GL_FLOAT, 0, &colors[0]);
	glNormalPointer(GL_FLOAT, 0, &normals[0]);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, 3 * trisN, GL_UNSIGNED_INT, &tris[0]);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

void TM::RotateAboutArbitraryAxis(V3 O, V3 a, float angled)
{
	for (int vi = 0; vi < vertsN; ++vi)
	{
		verts[vi] = verts[vi].RotateThisPointAboutArbitraryAxis(O, a, angled);
		normals[vi] = normals[vi].RotateThisPointAboutArbitraryAxis(O, a, angled);
	}
}

void TM::Translate(V3 tv)
{
	for (int vi = 0; vi < vertsN; ++vi)
	{
		verts[vi] = verts[vi] + tv;
	}
}

void TM::Scale(float scf)
{
	for (int vi = 0; vi < vertsN; ++vi)
	{
		verts[vi] = verts[vi] * scf;
	}
}

void TM::LoadModelBin(char* fname)
{
	ifstream ifs(fname, ios::binary);
	if (ifs.fail())
	{
		cerr << "INFO: cannot open file: " << fname << endl;
		return;
	}

	ifs.read((char*)&vertsN, sizeof(int));
	char yn;
	ifs.read(&yn, 1); // always xyz
	if (yn != 'y')
	{
		cerr << "INTERNAL ERROR: there should always be vertex xyz data" << endl;
		return;
	}
	if (!verts.empty())
		verts.clear();
	verts.resize(vertsN);

	ifs.read(&yn, 1); // cols 3 floats
	if (!colors.empty())
		colors.clear();
	if (yn == 'y')
	{
		colors.resize(vertsN);
	}

	ifs.read(&yn, 1); // normals 3 floats
	if (!normals.empty())
		normals.clear();
	if (yn == 'y')
	{
		normals.resize(vertsN);
	}

	ifs.read(&yn, 1); // texture coordinates 2 floats

	if (!vertST.empty())
		vertST.clear();

	if (yn == 'y')
	{
		vertST.resize(vertsN * 2);
	}

	ifs.read((char*)&verts[0], vertsN * 3 * sizeof(float)); // load verts

	if (colors.size() == vertsN)
	{
		ifs.read((char*)&colors[0], vertsN * 3 * sizeof(float)); // load cols
	}

	if (normals.size() == vertsN)
		ifs.read((char*)&normals[0], vertsN * 3 * sizeof(float)); // load normals

	if (vertST.size() != 0)
		ifs.read((char*)&vertST[0], vertsN * 2 * sizeof(float)); // load texture coordinates

	ifs.read((char*)&trisN, sizeof(int));
	if (tris.size() != 0)
		tris.clear();
	tris.resize(trisN * 3);
	ifs.read((char*)&tris[0], trisN * 3 * sizeof(unsigned int)); // read tiangles

	ifs.close();

	cerr << "INFO: loaded " << vertsN << " verts, " << trisN << " tris from " << endl << "      " << fname << endl;
	cerr << "      xyz " << ((colors.size() == 0) ? "rgb " : "") << ((normals.size() == 0) ? "nxnynz " : "") << (
		(vertST.size() == 0) ? "tcstct " : "") << endl;
}

AABB TM::ComputeAABB()
{
	AABB aabb(verts[0]);
	for (int vi = 0; vi < vertsN; ++vi)
	{
		aabb.AddPoint(verts[vi]);
	}
	return aabb;
}

float TM::ComputeSBBR(V3 c)
{
	float ret = 0.0f;
	for(auto v:verts)
	{
		ret = max((c - v).Length(), ret);
	}
	return ret;
}

void TM::PositionAndSize(V3 tmC, float tmSize)
{
	AABB aabb = ComputeAABB();
	V3 oldC = aabb.GetCenter();
	float oldSize = aabb.GetDiagnoalLength();

	Translate(V3(0.0f) - oldC);
	Scale(tmSize / oldSize);
	Translate(tmC);
}

V3 TM::GetCenter()
{
	AABB aabb(verts[0]);
	aabb = ComputeAABB();
	return aabb.GetCenter();
}

void TM::RayTracing(PPC* ppc, FrameBuffer* fb)
{
	for (int v = 0; v < fb->h; ++v)
	{
		if(GlobalVariables::Instance()->isDBGRaytracing)
		{
			fb->DrawRectangle(0, v, fb->w - 1, v + 1, 0xFFFF0000);
			fb->redraw();
			Fl::check();

			fb->DrawRectangle(0, v, fb->w - 1, v + 1, 0xFFFFFFFF);
		}

		for (int u = 0; u < fb->w; ++u)
		{
			for (int ti = 0; ti < trisN; ++ti)
			{
				auto index0 = tris[3 * ti + 0], index1 = tris[3 * ti + 1], index2 = tris[3 * ti + 2];
				V3 p0 = verts[index0];
				V3 p1 = verts[index1];
				V3 p2 = verts[index2];
				auto [a, b, c, w] = RayTriangleIntersect(ppc->C, ppc->GetRay(u, v), p0, p1, p2);

				// pruning branches
				if (a < 0.0f || b < 0.0f || c < 0.0f || w < 0.0f)
					continue;
				if (!fb->DepthTest(u, v, w))
					continue;

				// Shading
				V3 p = p0 * a + p1 * b + p2 * c;
				V3 pc = colors[index0] * a + colors[index1] * b + colors[index2] * c;
				V3 pn = normals[index0] * a + normals[index1] * b + normals[index2] * c;
				float s = vertST[index0 * 2 + 0] * a + vertST[index1 * 2 + 0] * b + vertST[index2 * 2 + 0] * c;
				float t = vertST[index0 * 2 + 1] * a + vertST[index1 * 2 + 1] * b + vertST[index2 * 2 + 1] * c;
				PointProperty pp(p, pc, pn, s, t);
				auto [color, alpha] = Shading(ppc, fb, u, v, w, pp);

				// alpha blending 
				if (!FloatEqual(alpha, 1.0f))
				{
					V3 bgC(0.0f);
					bgC.SetColor(fb->Get(u, v));
					color = color * alpha + bgC * (1.0f - alpha);
				}

				fb->DrawPoint(u, v, color.GetColor());
			}
		}
	}
}

tuple<V3, float> TM::Shading(PPC* ppc, FrameBuffer* fb, int u, int v, float w, PointProperty& pp, V3 dn)
{
	V3 color(0.0f); float alpha = 1.0f;
	if (fb->textures.find(tex) != fb->textures.end())
	{
		// s and t in (0.0f,1.0f)
		float s = Clamp(Fract(pp.s), 0.0f, 1.0f);
		float t = Clamp(Fract(pp.t), 0.0f, 1.0f);

		pp.c = fb->LookupColor(tex, s, t, alpha, pixelSz);
	}

	// environment mapping
	if (isEnvMapping)
	{
		auto [envColor, envEffect] = EnvMapping(ppc, fb, GlobalVariables::Instance()->curScene->cubemap.get(), pp.p, pp.n, dn);

		envEffect = isShowObjColor ? envEffect : 1.0f;
		pp.c = ClampColor(pp.c * (1.0f - envEffect) + envColor * envEffect);
	}

	if (GlobalVariables::Instance()->isRenderProjectedTexture)
	{
		V3 c(0.0f);
		float a = 0.0f;
		if (IsPixelInProjection(u, v, w, c, a))
			pp.c = pp.c * (1.0f - a) + c * a;
	}

	// Per pixel lighting 
	if (GlobalVariables::Instance()->isLight)
		color = Light(ppc, pp, u, v, w);
	else
		color = pp.c;

	return tuple<V3, float>(color, alpha);
}

void TM::Light(V3 mc, V3 L, PPC* ppc)
{
	for (int vi = 0; vi < vertsN; ++vi)
	{
		float ka = 0.5f;
		float kd = (L - verts[vi]).UnitVector() * normals[vi].UnitVector();
		float ks = (ppc->C - verts[vi]).UnitVector() * (L - verts[vi]).UnitVector().Reflect(normals[vi].UnitVector());
		kd = max(kd, 0.0f);
		ks = pow(max(ks, 0.0f), 8);
		colors[vi] = mc * (ka + (1.0f - ka) * kd) + ks;
	}
}

V3 TM::Light(PPC* ppc, PointProperty& pp, int u, int v, float w)
{
	V3 ret(0.0f);
	auto gv = GlobalVariables::Instance();
	if (gv->curScene->lightPPCs.empty())
		return pp.c;

	float ka = 0.2f;
	float kd = 0.0f, ks = 0.0f;
	float sd = 1.0f;
	for (size_t li = 0; li < gv->curScene->lightPPCs.size(); ++li)
	{
		auto ppc2 = gv->curScene->lightPPCs[li];

		// Phong
		kd += max((ppc2->C - pp.p).UnitVector() * pp.n.UnitVector(), 0.0f);
		float liks = (ppc->C - pp.p).UnitVector() * (ppc2->C - pp.p).UnitVector().Reflect(pp.n.UnitVector());
		ks += pow(max(liks, 0.0f), 200);

		// Shadow
		if (gv->isShadow && !gv->curScene->shadowMaps.empty())
		{
			auto SM = gv->curScene->shadowMaps[li];
			float uf = static_cast<float>(u), vf = static_cast<float>(v), z = w;
			V3 v2 = HomographMapping(V3(uf, vf, z), ppc, ppc2.get());
			if (v2[2] < 0.0f)
				continue;

			// compare shadow maps w
			float eps = 0.15f;
			if (SM->GetZ(v2[0], v2[1]) - v2[2] > eps)
			{
				sd *= 0.2f;
			}
		}
	}

	ka = Clamp(ka, 0.0f, 1.0f);
	kd = Clamp(kd, 0.0f, 1.0f);
	ks = Clamp(ks, 0.0f, 1.0f);

	ret = pp.c * (ka + (1.0f - ka) * kd) + V3(1.0f) * ks;
	ret = ret * sd; // shadow
	return ret;
}

bool TM::ComputeShadowEffect(PPC* ppc, int u, int v, float z, float& sdEffect)
{
	bool isInSS = false;
	sdEffect = 1.0f; // shadow effect
	auto gv = GlobalVariables::Instance();
	if (gv->curScene->shadowMaps.empty())
		return isInSS;

	float uf = static_cast<float>(u) + 0.5f, vf = static_cast<float>(v) + 0.5f;
	// Check for all shadow maps
	for (size_t li = 0; li < gv->curScene->lightPPCs.size(); ++li)
	{
		auto ppc1 = ppc;
		auto ppc2 = gv->curScene->lightPPCs[li];
		auto SM = gv->curScene->shadowMaps[li];

		V3 v2 = HomographMapping(V3(uf, vf, z), ppc1, ppc2.get());

		if (v2[2] < 0.0f)
			continue;

		// compare shadow maps w
		float eps = 0.15f;
		if (SM->GetZ(v2[0], v2[1]) - v2[2] > eps)
		{
			// in shadow
			isInSS = true;
			sdEffect *= 0.2f;
		}
	}

	return isInSS;
}

bool TM::IsPixelInProjection(int u, int v, float z, V3& color, float& alpha)
{
	auto gv = GlobalVariables::Instance();
	float uf = static_cast<float>(u) + 0.5f, vf = static_cast<float>(v) + 0.5f;

	auto ppc1 = gv->curScene->ppc;
	auto ppc2 = gv->curScene->projectPPC;
	auto projFB = gv->curScene->fbp;
	string projTexName = gv->projectedTextureName;

	if (!ppc1 || !ppc1 || !projFB)
		return false;

	V3 v2 = HomographMapping(V3(uf, vf, z), ppc1, ppc2);

	if (v2[2] < 0.0f)
		return false;

	AABB aabb(v2);
	if (!aabb.Clip2D(0, projFB->w - 1, 0, projFB->h - 1))
		return false;

	float eps = 0.01f;

	if (projFB->GetZ(v2[0], v2[1]) - v2[2] <= eps)
	{
		unsigned int c = projFB->Get(v2[0], v2[1]);
		color.SetColor(c);
		unsigned char* rgba = (unsigned char*)&c;
		alpha = static_cast<float>(rgba[3]) / 255.0f;
		return true;
	}

	return false;
}

// envEffect (0,1)
// 1 means should igore point color
tuple<V3, float> TM::EnvMapping(PPC* ppc, FrameBuffer* fb, CubeMap* cubemap, V3 p, V3 n, V3 dn)
{
	V3 c(0.0f);
	float envEffect = 0.0f;
	if (!cubemap)
		return tuple<V3, float>(c,envEffect);

	envEffect = 0.4f;
	auto gv = GlobalVariables::Instance();
	V3 viewDir = (ppc->C - p).UnitVector();

	if (isRefraction)
		viewDir = viewDir.Refract(n, gv->refractRatio);
	else
		viewDir = viewDir.Reflect(n);

	// Check intersections
	auto sceneBBs = gv->curScene->sceneBillboard;
	float distance = 0.0f;
	V3 bbColor(0.0f);
	float alpha = 0.0f;
	EnvBBIntersection(sceneBBs, p, viewDir, distance, bbColor, alpha);
	EnvBBIntersection(reflectorBB, p, viewDir, distance, bbColor, alpha);
	bbColor = bbColor * alpha;

	// intersect with bb
	if (!FloatEqual(distance, 0.0f))
	{
		distance = 1.0f / distance;
		float disAttenauation = max(pow(distance * 10.0f, 2), 1.0f);
		// envEffect = Clamp(1.0f / disAttenauation,0.0f,1.0f);
		return tuple<V3, float>(bbColor,0.3f);
	}

	// Chose mipmap level, use dn to approximate pixelsz
	float pxSz = dn != V3(0.0f) ? abs((n + dn).UnitVector() * n.UnitVector()) * static_cast<float>(ppc->h) * 0.5f : -1;
	return tuple<V3, float>(cubemap->LookupColor(viewDir, pxSz), envEffect);
}

int TM::EnvBBIntersection(vector<shared_ptr<BillBoard>> bbs, V3 p, V3 viewDir, float& distance, V3& bbColor,
                          float& alpha)
{
	int ret = 0;
	for (auto b : bbs)
	{
		float t = 0.0f;
		if (!b->Intersect(p, viewDir, t))
			continue;

		ret = 1;
		t = 1.0 / t;

		// Find the closest Billboard
		if (distance < t)
		{
			distance = t;

			// update closest color
			t = 1.0f / t;
			V3 pBB = p + viewDir * t;
			bbColor = b->mesh->tex.empty() ? b->GetColor(pBB, alpha) : b->GetColor(b->fbTexture.get(), pBB, alpha);

			// intersect 0 alpha part of billbard
			if (FloatEqual(alpha, 0.0f))
				distance = 0.0f;
		}
	}

	return ret;
}

V3 TM::ClampColor(V3 color)
{
	V3 ret(0.0f);
	ret[0] = Clamp(color[0], 0.0f, 1.0f);
	ret[1] = Clamp(color[1], 0.0f, 1.0f);
	ret[2] = Clamp(color[2], 0.0f, 1.0f);
	return ret;
}

V3 TM::HomographMapping(V3 uvw, PPC* ppc1, PPC* ppc2)
{
	// Current image plane ppc matrix
	M33 abc1;
	abc1.SetColumn(0, ppc1->a);
	abc1.SetColumn(1, ppc1->b);
	abc1.SetColumn(2, ppc1->c);

	M33 abc2;
	abc2.SetColumn(0, ppc2->a);
	abc2.SetColumn(1, ppc2->b);
	abc2.SetColumn(2, ppc2->c);
	auto abc2Inv = abc2.Inverse();

	auto qC = abc2Inv * (ppc1->C - ppc2->C);
	auto qM = abc2Inv * abc1;

	float w1 = 1.0f / uvw[2];
	V3 px = V3(uvw[0], uvw[1], 1.0f) * w1;
	float w2 = 1.0f / (qC[2] + qM[2] * px);
	float u2 = (qC[0] + qM[0] * px) * w2;
	float v2 = (qC[1] + qM[1] * px) * w2;

	return V3(u2, v2, w2);
}

void TM::SetAllPointsColor(V3 color)
{
	for (int vi = 0; vi < vertsN; ++vi)
	{
		colors[vi] = color;
	}
}


void TM::SphereMorph(V3 c, float r, float fract)
{
	fract = Clamp(fract, 0.0f, 1.0f);
	if (staticVerts.empty())
		staticVerts = verts;

	if (staticNorms.empty())
		staticNorms = normals;

	for (int vi = 0; vi < vertsN; ++vi)
	{
		V3 vp = staticVerts[vi];
		V3 dis = vp - c;
		dis = dis.UnitVector() * r;
		verts[vi] = vp * (1.0f - fract) + (c + dis) * fract;

		V3 n = staticNorms[vi];
		V3 newn = dis.UnitVector();
		normals[vi] = n * (1.0f - fract) + newn * fract;
	}
}

void TM::WaterAnimation(float t)
{
	if (staticVerts.empty())
		staticVerts = verts;

	// TM animation according to time t
	for (int vi = 0; vi < vertsN; ++vi)
	{
		// Normal not correct animation
		V3 vp = verts[vi];
		float scalef = vp[1];

		float a = exp(-0.005f * t);
		vp[0] = vp[0] * (1.0f + a * 0.15f * sin(0.1f * scalef + 0.5f * t));
		vp[2] = vp[2] * (1.0f + a * 0.15f * sin(0.1f * scalef + 0.5f * t));

		// commit result
		verts[vi] = vp;
	}
}

tuple<float, float, float, float> TM::RayTriangleIntersect(V3 C, V3 ray, V3 p0, V3 p1, V3 p2)
{
	M33 m;
	m.SetColumn(0, p0);
	m.SetColumn(1, p1);
	m.SetColumn(2, p2);
	m = m.Inverse();
	V3 q1 = m * C, q2 = m * ray;

	float w = (1.0f - V3(1.0f) * q1) / (V3(1.0f) * q2);
	V3 abc = q1 + q2 * w;

	return tuple<float, float, float, float>(abc[0], abc[1], abc[2], 1.0f / w);
}

tuple<PointProperty, float> TM::RayMeshIntersect(V3 C, V3 ray)
{
	float closestW = 0.0f;
	PointProperty closestPP(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	for (int ti = 0; ti < trisN; ++ti)
	{
		auto index0 = tris[3 * ti + 0], index1 = tris[3 * ti + 1], index2 = tris[3 * ti + 2];
		V3 p0 = verts[index0];
		V3 p1 = verts[index1];
		V3 p2 = verts[index2];
		auto[a, b, c, w] = RayTriangleIntersect(C, ray, p0, p1, p2);

		// pruning branches
		if (a < 0.0f || b < 0.0f || c < 0.0f || w < 0.0f)
			continue;
		if (closestW > w)
			continue;

		// Update Closest Point property
		closestW = w;
		V3 p = p0 * a + p1 * b + p2 * c;
		V3 pc = colors[index0] * a + colors[index1] * b + colors[index2] * c;
		V3 pn = normals[index0] * a + normals[index1] * b + normals[index2] * c;
		float s = vertST[index0 * 2 + 0] * a + vertST[index1 * 2 + 0] * b + vertST[index2 * 2 + 0] * c;
		float t = vertST[index0 * 2 + 1] * a + vertST[index1 * 2 + 1] * b + vertST[index2 * 2 + 1] * c;
		closestPP = PointProperty(p, pc, pn, s, t);
	}

	return tuple<PointProperty, float>(closestPP, closestW);
}

TM::~TM()
{
}
