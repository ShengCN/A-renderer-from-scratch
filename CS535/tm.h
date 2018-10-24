#pragma once
#include "v3.h"
#include "framebuffer.h"
#include "AABB.h"
#include <vector>
#include "cubemap.h"
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
	vector<V3> verts, colors, normals, staticVerts;		// vertices and colors
	vector<float> tcs;
	vector<unsigned int>  tris;		// indices
	int vertsN, trisN;
	std::string tex;
	int pixelSz;		// approximate projected aabb size
	bool isEnvMapping;

	TM() :vertsN(0), trisN(0), pixelSz(0), isEnvMapping(false) {};
	void SetRectangle(V3 O, float rw, float rh);
	void SetTriangle(PointProperty p0, PointProperty p1, PointProperty p2);
	void SetQuad(PointProperty p0, PointProperty p1, PointProperty p2, PointProperty p3);
	void SetBillboard(V3 O, V3 n, V3 up, float sz);
	void SetText(std::string tf);
	void Allocate();
	void RenderPoints(PPC *ppc, FrameBuffer *fb);
	void RenderWireFrame(PPC *ppc, FrameBuffer *fb);
	void RenderFill(PPC *ppc, FrameBuffer *fb);
	void RenderFillZ(PPC *ppc, FrameBuffer *fb); // only draw z buffer
	void RenderAABB(PPC *ppc, FrameBuffer *fb);
	void RotateAboutArbitraryAxis(V3 O, V3 a, float angled);
	void Translate(V3 tv);
	void Scale(float scf);		// normalize size to some scf
	void LoadModelBin(char *fname);
	AABB ComputeAABB();
	void PositionAndSize(V3 tmC, float tmSize);
	V3 GetCenter();

	// Shading
	V3 Shading(PPC *ppc, FrameBuffer *fb, int u, int v, float w, PointProperty pp, float &alpha);
	void Light(V3 mc, V3 L, PPC *ppc);	  // Per vertex light
	V3 Light(PPC *ppc, PointProperty pp, int u, int v, float w); // Per pixel  light 
	bool ComputeShadowEffect(PPC* ppc, int u, int v, float z, float &sdEffect);
	bool IsPixelInProjection(int u, int v, float z, V3 &color, float &alpha);
	V3 EnvMapping(PPC *ppc, FrameBuffer *fb, CubeMap *cubemap, V3 p, V3 n);
	V3 ClampColor(V3 color);
	V3 HomographMapping(V3 uvw, PPC* ppc1, PPC* ppc2);


	// Morphing
	void SphereMorph(V3 c,float r, float fract);
	void WaterAnimation(float t);

	~TM();
};

