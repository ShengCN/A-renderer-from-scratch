#include <fstream>
#include <algorithm>

#include "TM.h"
#include "MathTool.h"

using namespace std;

void TM::SetRectangle(V3 O, float rw, float rh)
{
	vertsN = 4; trisN = 2;
	Allocate();

	verts[0] = O + V3(-rw / 2.0f,  rh / 2.0f, 0.0f);
	verts[1] = O + V3(-rw / 2.0f, -rh / 2.0f, 0.0f);
	verts[2] = O + V3( rw / 2.0f, -rh / 2.0f, 0.0f);
	verts[3] = O + V3( rw / 2.0f,  rh / 2.0f, 0.0f);

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
	
	for(int vi = 0; vi < vertsN; ++vi)
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

	tcs[0] = p0.s;
	tcs[1] = p0.t;
	tcs[2] = p1.s;
	tcs[3] = p1.t;
	tcs[4] = p2.s;
	tcs[5] = p2.t;
	tcs[6] = p3.s;
	tcs[7] = p3.t;

	tris[0] = 0;
	tris[1] = 1;
	tris[2] = 2;

	tris[3] = 2;
	tris[4] = 3;
	tris[5] = 0;
}

void TM::SetText(std::string tf)
{
	tex = tf;
}

void TM::Allocate()
{
	verts.resize(vertsN);
	colors.resize(vertsN);
	normals.resize(vertsN);
	tcs.resize(2 * vertsN);
	tris.resize(3 * trisN);		// each triangle has three topological indexs
}

void TM::RenderPoints(PPC* ppc, FrameBuffer* fb)
{
	for(int vi = 0; vi < vertsN; ++vi)
	{
		fb->Draw3DPoint(ppc,verts[vi], 0xFF000000,7);
	}
}

void TM::RenderWireFrame(PPC* ppc, FrameBuffer* fb)
{
	for(int ti = 0; ti < trisN; ++ti)
	{
		// For each triangle, draw three line segments
		for(int ei = 0; ei < 3; ++ei)
		{
			int vi0 = tris[ti * 3 + ei];
			int vi1 = tris[ti * 3 + (ei + 1) % 3];
			fb->Draw3DSegment(ppc, verts[vi0], colors[vi0], verts[vi1], colors[vi1]);
		}
	}
}

void TM::RenderFill(PPC* ppc, FrameBuffer* fb)
{
	for (int ti = 0; ti < trisN; ++ti)
	{
		int vi0 = tris[ti * 3 + 0];
		int vi1 = tris[ti * 3 + 1];
		int vi2 = tris[ti * 3 + 2];

		fb->Draw3DTriangle(ppc, verts[vi0], colors[vi0], 
								verts[vi1], colors[vi1],
								verts[vi2], colors[vi2]);
	}
}

void TM::RenderFillTexture(PPC* ppc, FrameBuffer* fb)
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

	if (tcs.size() == verts.size() * 2)
	{
		for (int ti = 0; ti < trisN; ++ti)
		{
			int vi0 = tris[ti * 3 + 0];
			int vi1 = tris[ti * 3 + 1];
			int vi2 = tris[ti * 3 + 2];

			PointProperty p0(verts[vi0], colors[vi0], normals[vi0], tcs[vi0 * 2], tcs[vi0 * 2 + 1]);
			PointProperty p1(verts[vi1], colors[vi1], normals[vi1], tcs[vi1 * 2], tcs[vi1 * 2 + 1]);
			PointProperty p2(verts[vi2], colors[vi2], normals[vi2], tcs[vi2 * 2], tcs[vi2 * 2 + 1]);

			// According to loD, do trilinear in texture look up
			fb->Draw3DTriangleTexture(ppc, p0, p1, p2, tex, pixelSz);
		}
	}
	else
	{
		// no texture
		RenderFill(ppc, fb);
	}
}

void TM::RenderFillZ(PPC* ppc, FrameBuffer* fb)
{
	for (int ti = 0; ti < trisN; ++ti)
	{
		int vi0 = tris[ti * 3 + 0];
		int vi1 = tris[ti * 3 + 1];
		int vi2 = tris[ti * 3 + 2];

		fb->Draw3DTriangle(ppc, verts[vi0], verts[vi1], verts[vi2]);
	}
}

void TM::RenderAABB(PPC* ppc, FrameBuffer* fb)
{
	auto aabb = ComputeAABB();
	V3 x(1.0f, 0.0f, 0.0f), y(0.0f, 1.0f, 0.0f), z(0.0f, 0.0f, 1.0f);
	V3 lc = aabb.corners[0], rc = aabb.corners[1];	// left corner and right corner, left "smaller" than right
	float dx = (rc - lc)*x;
	float dy = (rc - lc)*y;
	float dz = (rc - lc)*z;

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

void TM::RotateAboutArbitraryAxis(V3 O, V3 a, float angled)
{
	for(int vi = 0; vi < vertsN; ++vi)
	{
		verts[vi] = verts[vi].RotateThisPointAboutArbitraryAxis(O, a, angled);
	}
}

void TM::Translate(V3 tv)
{
	for(int vi = 0; vi < vertsN; ++vi)
	{
		verts[vi] = verts[vi] + tv;
	}
}

void TM::Scale(float scf)
{
	for(int vi = 0; vi < vertsN; ++vi)
	{
		verts[vi] = verts[vi] * scf;
	}
}

void TM::LoadModelBin(char* fname)
{
	ifstream ifs(fname, ios::binary);
	if (ifs.fail()) {
		cerr << "INFO: cannot open file: " << fname << endl;
		return;
	}

	ifs.read((char*)&vertsN, sizeof(int));
	char yn;
	ifs.read(&yn, 1); // always xyz
	if (yn != 'y') {
		cerr << "INTERNAL ERROR: there should always be vertex xyz data" << endl;
		return;
	}
	if (verts.size()!=0)
		verts.clear();
	verts.resize(vertsN);

	ifs.read(&yn, 1); // cols 3 floats
	if (colors.size()!=0)
		colors.clear();
	if (yn == 'y') {
		colors.resize(vertsN);
	}

	ifs.read(&yn, 1); // normals 3 floats
	if (normals.size() != 0)
		normals.clear();
	if (yn == 'y') {
		normals.resize(vertsN);
	}

	ifs.read(&yn, 1); // texture coordinates 2 floats
	
	if (tcs.size() != 0)
		tcs.clear();
	
	if (yn == 'y') {
		tcs.resize(vertsN * 2);
	}

	ifs.read((char*)&verts[0], vertsN * 3 * sizeof(float)); // load verts

	if (colors.size() == vertsN) {
		ifs.read((char*)&colors[0], vertsN * 3 * sizeof(float)); // load cols
	}

	if (normals.size() == vertsN)
		ifs.read((char*)&normals[0], vertsN * 3 * sizeof(float)); // load normals

	if (tcs.size() != 0)
		ifs.read((char*)&tcs[0], vertsN * 2 * sizeof(float)); // load texture coordinates

	ifs.read((char*)&trisN, sizeof(int));
	if (tris.size() != 0)
		tris.clear();
	tris.resize(trisN * 3);
	ifs.read((char*)&tris[0], trisN * 3 * sizeof(unsigned int)); // read tiangles

	ifs.close();

	cerr << "INFO: loaded " << vertsN << " verts, " << trisN << " tris from " << endl << "      " << fname << endl;
	cerr << "      xyz " << ((colors.size()==0) ? "rgb " : "") << ((normals.size()==0) ? "nxnynz " : "") << ((tcs.size()==0) ? "tcstct " : "") << endl;
}

AABB TM::ComputeAABB()
{
	AABB aabb(verts[0]);
	for(int vi = 0; vi< vertsN; ++vi)
	{
		aabb.AddPoint(verts[vi]);
	}
	return aabb;
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

void TM::Light(V3 mc, V3 L, PPC *ppc)
{
	for(int vi = 0; vi < vertsN; ++vi)
	{
		float ka = 0.5f;
		float kd = (L - verts[vi]).UnitVector() * normals[vi].UnitVector();
		float ks = (ppc->C - verts[vi]).UnitVector() * (L-verts[vi]).UnitVector().Reflect(normals[vi].UnitVector());
		kd = max(kd, 0.0f);
		ks = pow(max(ks, 0.0f), 8);
		colors[vi] = mc * (ka + (1.0f - ka)*kd) + ks;
	}
}

TM::~TM()
{
}
