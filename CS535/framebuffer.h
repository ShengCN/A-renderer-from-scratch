#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>
#include <vector>
#include <unordered_map>
#include <string>
#include "ppc.h"
#include "v3.h"
using std::vector;
using std::unordered_map;
struct PointProperty;
struct TextureInfo
{
	vector<unsigned int> texture;
	int w, h;
	TextureInfo() :w(0), h(0){};
};

class FrameBuffer : public Fl_Gl_Window {
public:
	unsigned int *pix;	// pixel 
	float *zb;			// z buffer
	unordered_map<std::string, vector<TextureInfo>> textures; // use file name as index, different Lod, 0 is default
	bool depthTest, lodTexture;
	int w, h;
	std::vector<V3> Ls;		// light position, prepare for mulitple light

	FrameBuffer(int u0, int v0, int _w, int _h);
	void draw();
	void KeyboardHandle();
	int handle(int guievent);
	void SetBGR(unsigned int bgr);
	void SetGuarded(int u, int v, unsigned int color);
	void LoadTiff(const char* fname);
	void SaveAsTiff(const char* fname);
	void SaveTextureAsTiff(string fname, const string textureName, int loD);
	int ClipToScreen(int& u0, int& v0, int& u1, int& v1);
	int ClipToScreen(float& u0, float& v0, float& u1, float& v1);
	bool IsInScreen(int u, int v);
	void Clear(unsigned int bgr, float z0);
	bool Visible(int u, int v, float z);
	float GetZ(int u, int v);
	unsigned int Get(int u, int v);
	bool LoadTex(const std::string texFile);
	bool IsPixelInShadow(int u, int v);		// iterate all lights in scene

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
	void Draw3DTriangleTexture(PPC *ppc, PointProperty p0, PointProperty p1, PointProperty p2, const std::string texFile, int pixelSz = -1);
	void DrawPPC(PPC* wPPC, PPC* tPPC, float vf);	// visualize target PPC using wPPC
	void VisualizeCurrView(PPC *ppc0, float currf, PPC *ppc1, FrameBuffer *fb1);
	void VisualizeCurrView3D(PPC *ppc0, PPC *ppc1, FrameBuffer *fb1); 
	V3 LookupColor(std::string texFile, float s, float t);
	V3 LookupColor(std::string texFile, float s, float t, float &alpha, int pixelSz = -1);
	V3 BilinearLookupColor(TextureInfo &tex, float s, float t, float &alpha);
	V3 Light(PointProperty pp, V3 L, PPC *ppc);	// point property, ppc
	V3 Light(PointProperty pp, std::vector<V3> Ls, PPC *ppc);

	// Texture downsampling
	void PrepareTextureLoD(string texFile);

private:
	void Set(int u, int v, int color);
	bool InsideTriangle(V3 p, V3 v1, V3 v2, V3 v3);
};

