#pragma once
#include "v3.h"
#include "framebuffer.h"

// Triangle Mesh

class TM
{
public:
	V3 *verts, *colors, *normals;		// vertices and colors
	unsigned int *tris;		// topological index
	int vertsN, trisN;
	
	TM() :verts(nullptr), colors(nullptr), tris(nullptr), vertsN(0), trisN(0){};
	void SetRectangle(V3 O, float rw, float rh);
	void Allocate();
	void RenderPoints(PPC *ppc, FrameBuffer *fb);
	void RenderWireFrame(PPC *ppc, FrameBuffer *fb);
	void RotateAboutArbitraryAxis(V3 O, V3 a, float angled);
	void LoadBin(char *fname);
	~TM();
};

