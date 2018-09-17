#pragma once
#include "v3.h"
#include "framebuffer.h"
#include "AABB.h"

// Triangle Mesh

class TM
{
public:
	V3 *verts, *colors, *normals;		// vertices and colors
	unsigned int *tris;		// topological index
	int vertsN, trisN;
	
	TM() :verts(nullptr), colors(nullptr), tris(nullptr), vertsN(0), trisN(0){};
	void SetRectangle(V3 O, float rw, float rh);
	void SetTriangle(V3 p0, V3 c0, V3 p1, V3 c1, V3 p2, V3 c2);
	void Allocate();
	void RenderPoints(PPC *ppc, FrameBuffer *fb);
	void RenderWireFrame(PPC *ppc, FrameBuffer *fb);
	void Render(PPC *ppc, FrameBuffer *fb);
	void RotateAboutArbitraryAxis(V3 O, V3 a, float angled);
	void Translate(V3 tv);
	void Scale(float scf);		// normalize size to some scf
	void LoadBin(char *fname);
	AABB ComputeAABB();
	void PositionAndSize(V3 tmC, float tmSize);
	V3 GetCenter();

	~TM();
};

