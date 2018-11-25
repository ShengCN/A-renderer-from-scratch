#pragma once
#include "TM.h"
#include "m33.h"

class BillBoard
{
public:
	shared_ptr<TM> mesh;
	shared_ptr<FrameBuffer> fbTexture;
	GLuint texID;

	BillBoard();
	~BillBoard();

	void SetBillboard(V3 O, V3 n, V3 up, float sz, float s = 1.0f, float t = 1.0f);
	void GPUsetBBtexture();
	bool Intersect(V3 p, V3 d, float & t);
	bool InsideBillboard(V3 p);

	// Get Billboard corners
	M33 GetCorners();

	// From Framebuffer
	V3 GetColor(V3 p, float &alpha);

	// From texture
	V3 GetColor(FrameBuffer *fb, V3 p);
	V3 GetColor(FrameBuffer *fb, V3 p, float &alpha);

	void RenderBB(PPC *ppc, FrameBuffer *fb);
private:
	void GetST(V3 p, float &s, float &t);
};

