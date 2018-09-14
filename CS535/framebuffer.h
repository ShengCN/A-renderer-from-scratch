#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>

#include "ppc.h"
#include "v3.h"

class FrameBuffer : public Fl_Gl_Window {
public:
	unsigned int *pix;	// pixel 
	float *zb;			// z buffer
	int w, h;
	FrameBuffer(int u0, int v0, int _w, int _h);
	void draw();
	void KeyboardHandle();
	int handle(int guievent);
	void SetBGR(unsigned int bgr);
	void SetGuarded(int u, int v, unsigned int color);
	void LoadTiff(char* fname);
	void SaveAsTiff(char* fname);
	int ClipToScreen(int& u0, int& v0, int& u1, int& v1);
	int ClipToScreen(float& u0, float& v0, float& u1, float& v1);
	bool IsInScreen(int u, int v);
	void Clear(unsigned int bgr, float z0);
	bool Visible(int u, int v, float z);

	// Draw something
	void DrawSegment(V3 p0, V3 c0, V3 p1, V3 c1);
	void Draw3DSegment(PPC *ppc, V3 p1, V3 c1, V3 p2, V3 c2);
	void DrawRectangle(int u0, int v0, int u1, int v1, unsigned int color);
	void DrawCircle(int u0, int v0, int r, unsigned int color);
	void DrawEllipse(int u0, int v0, float r0, float r1, unsigned int color);
	void DrawPoint(int u, int v, unsigned int color);
	void Draw3DPoint(PPC* ppc, V3 p, unsigned int color, int pointSize);
	void Draw3DTriangle(PPC* ppc, V3 p1, V3 p2, V3 p3, V3 color);
	void Draw3DTriangle(PPC* ppc, V3 p0, V3 c0, V3 p1, V3 c1, V3 p2, V3 c2);
private:
	void Set(int u, int v, int color);
	bool InsideTriangle(V3 p, V3 v1, V3 v2, V3 v3);
};