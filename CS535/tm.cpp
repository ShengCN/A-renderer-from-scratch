#include "TM.h"
#include <fstream>
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

void TM::SetTriangle(V3 p0, V3 c0, V3 p1, V3 c1, V3 p2, V3 c2)
{
	vertsN = 3;
	trisN = 1;
	Allocate();

	verts[0] = p0;
	verts[1] = p1;
	verts[2] = p2;

	colors[0] = c0;
	colors[1] = c1;
	colors[2] = c2;

	tris[0] = 0;
	tris[1] = 1;
	tris[2] = 2;
}

void TM::Allocate()
{
	verts = new V3[vertsN];
	colors = new V3[vertsN];
	normals = new V3[vertsN];
	tris = new unsigned int[3 * trisN];		// each triangle has three topological indexs
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

void TM::Render(PPC* ppc, FrameBuffer* fb)
{
	for (int ti = 0; ti < trisN; ++ti)
	{
		int vi0 = tris[ti * 3 + 0];
		int vi1 = tris[ti * 3 + 1];
		int vi2 = tris[ti * 3 + 2];
		fb->Draw3DTriangle(ppc, verts[vi0], colors[vi0], 
								verts[vi1], colors[vi1],
								verts[vi2], colors[vi2]);

//		fb->Draw3DTriangle(ppc, verts[vi0], verts[vi1], verts[vi2], V3(0.8f));
	}
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

void TM::LoadBin(char* fname)
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
	if (verts)
		delete verts;
	verts = new V3[vertsN];

	ifs.read(&yn, 1); // cols 3 floats
	if (colors)
		delete colors;
	colors = 0;
	if (yn == 'y') {
		colors = new V3[vertsN];
	}

	ifs.read(&yn, 1); // normals 3 floats
	if (normals)
		delete normals;
	normals = 0;
	if (yn == 'y') {
		normals = new V3[vertsN];
	}

	ifs.read(&yn, 1); // texture coordinates 2 floats
	float *tcs = 0; // don't have texture coordinates for now
	if (tcs)
		delete tcs;
	tcs = 0;
	if (yn == 'y') {
		tcs = new float[vertsN * 2];
	}

	ifs.read((char*)verts, vertsN * 3 * sizeof(float)); // load verts

	if (colors) {
		ifs.read((char*)colors, vertsN * 3 * sizeof(float)); // load cols
	}

	if (normals)
		ifs.read((char*)normals, vertsN * 3 * sizeof(float)); // load normals

	if (tcs)
		ifs.read((char*)tcs, vertsN * 2 * sizeof(float)); // load texture coordinates

	ifs.read((char*)&trisN, sizeof(int));
	if (tris)
		delete tris;
	tris = new unsigned int[trisN * 3];
	ifs.read((char*)tris, trisN * 3 * sizeof(unsigned int)); // read tiangles

	ifs.close();

	cerr << "INFO: loaded " << vertsN << " verts, " << trisN << " tris from " << endl << "      " << fname << endl;
	cerr << "      xyz " << ((colors) ? "rgb " : "") << ((normals) ? "nxnynz " : "") << ((tcs) ? "tcstct " : "") << endl;

	delete[]tcs;

}

AABB TM::ComputeAABB()
{
	AABB aabb(0.0f);
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

TM::~TM()
{
	if (verts != nullptr)
		delete[] verts;
	if (verts != nullptr)
		delete[] colors;
	if (verts != nullptr)
		delete[] tris;
}
