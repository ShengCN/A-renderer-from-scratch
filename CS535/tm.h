#pragma once
#include "v3.h"
#include "framebuffer.h"
#include "AABB.h"
#include <memory>
#include <vector>
using std::vector;

struct PointProperty
{
	V3 p, c, n;		// position, color, normal
	float s, t;		// texture s, texture t
	PointProperty(V3 _p, V3 _c, V3 _n, float _s, float _t) :p(_p), c(_c), n(_n), s(_s), t(_t) {};
};

// Triangle Mesh
class TM
{
public:
	vector<V3> verts, colors, normals, texture;		// vertices and colors
	vector<unsigned int>  tris;		// topological index
	int vertsN, trisN;
	
	TM() :vertsN(0), trisN(0){};
	void SetRectangle(V3 O, float rw, float rh);
	void SetTriangle(PointProperty p0, PointProperty p1, PointProperty p2);
	void SetQuad(PointProperty p0, PointProperty p1, PointProperty p2, PointProperty p3);
	void Allocate();
	void RenderPoints(PPC *ppc, FrameBuffer *fb);
	void RenderWireFrame(PPC *ppc, FrameBuffer *fb);
	void RenderFill(PPC *ppc, FrameBuffer *fb);
	void RenderAABB(PPC *ppc, FrameBuffer *fb);
	void RotateAboutArbitraryAxis(V3 O, V3 a, float angled);
	void Translate(V3 tv);
	void Scale(float scf);		// normalize size to some scf
	void LoadBin(char *fname);
	AABB ComputeAABB();
	void PositionAndSize(V3 tmC, float tmSize);
	V3 GetCenter();

	~TM();
};

