#include "TM.h"


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

void TM::Allocate()
{
	verts = new V3[vertsN];
	colors = new V3[vertsN];
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

void TM::RotateAboutArbitraryAxis(V3 O, V3 a, float angled)
{
	for(int vi = 0; vi < vertsN; ++vi)
	{
		verts[vi] = verts[vi].RotateThisPointAboutArbitraryAxis(O, a, angled);
	}
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
