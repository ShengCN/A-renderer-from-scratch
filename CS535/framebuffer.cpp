#include "framebuffer.h"
#include "math.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include "scene.h"
#include "v3.h"
#include <iostream>

#include "tiffio.h"
#include "AABB.h"
#include "MathTool.h"
#include "m33.h"
// #define SCREEN_SPACE_INTERPOLATION
#define PERSPECTIVE_CORRECT_INTERPOLATION

FrameBuffer::FrameBuffer(int u0, int v0, int _w, int _h)
	: Fl_Gl_Window(u0, v0, _w, _h, nullptr)
{
	w = _w;
	h = _h;
	pix = new unsigned int[w * h];
	zb = new float[w*h];
}


void FrameBuffer::draw()
{
	glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pix);
}

int FrameBuffer::handle(int event)
{
	switch (event)
	{
	case FL_KEYBOARD:
		{
			KeyboardHandle();
			return 0;
		}
	default:
		break;
	}
	return 0;
}

void FrameBuffer::KeyboardHandle()
{
	int key = Fl::event_key();
	switch (key)
	{
	case ',':
		{
			cerr << "INFO: pressed ," << endl;
			break;
		}
	default:
		cerr << "INFO: do not understand keypress" << endl;
	}
}


void FrameBuffer::SetBGR(unsigned int bgr)
{
	// memset(pix, bgr, sizeof(unsigned int) * w * h);

	for (int u = 0; u < w; ++u)
	{
		for (int v = 0; v < h; ++v)
		{
			pix[(h - 1 - v)*w + u] = bgr;
		}
	}
}

void FrameBuffer::Set(int u, int v, int color)
{
	pix[(h - 1 - v) * w + u] = color;
}

bool FrameBuffer::InsideTriangle(V3 p, V3 v1, V3 v2, V3 v3)
{
	float x[3], y[3];
	x[1] = v1[0];
	x[2] = v2[0];
	y[1] = v1[1];
	y[2] = v2[1];

	float xCo = y[2] - y[1];
	float yCo = x[2] - x[1];
	float x1y2 = x[1] * y[2];
	float y1x2 = y[1] * x[2];
	float res1 = p[0] * xCo - p[1] * yCo - x1y2 + y1x2;
	float res2 = v3[0] * xCo - v3[1] * yCo - x1y2 + y1x2;

	return res1 * res2 >= 0.0f ? true : false;
}

float FrameBuffer::Fract(float n)
{
	return n - static_cast<int>(n);
}

void FrameBuffer::SetGuarded(int u, int v, unsigned int color)
{
	// clip to window 
	if (!IsInScreen(u, v))
		return;

	Set(u, v, color);
}

// load a tiff image to pixel buffer
void FrameBuffer::LoadTiff(const char* fname)
{
	TIFF* in = TIFFOpen(fname, "r");
	if (in == nullptr)
	{
		cerr << fname << " could not be opened" << endl;
		return;
	}

	int width, height;
	TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);
	if (w != width || h != height)
	{
		w = width;
		h = height;
		delete[] pix;
		pix = new unsigned int[w * h];
		size(w, h);
		glFlush();
		glFlush();
	}

	if (TIFFReadRGBAImage(in, w, h, pix, 0) == 0)
	{
		cerr << "failed to load " << fname << endl;
	}

	TIFFClose(in);
}

// save as tiff image
void FrameBuffer::SaveAsTiff(const char* fname)
{
	TIFF* out = TIFFOpen(fname, "w");

	if (out == nullptr)
	{
		cerr << fname << " could not be opened" << endl;
		return;
	}

	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 4);
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

	for (uint32 row = 0; row < (unsigned int)h; row++)
	{
		TIFFWriteScanline(out, &pix[(h - row - 1) * w], row);
	}

	TIFFClose(out);
}

int FrameBuffer::ClipToScreen(int& u0, int& v0, int& u1, int& v1)
{
	// Out of screen 
	if (u0 > w || v0 > h || u1 < 0 || v1 < 0)
		return 0;

	u0 = std::max(0, u0);
	v0 = std::max(0, v0);
	u1 = std::min(u1, w-1);
	v1 = std::min(v1, h-1);
	return 1;
}

int FrameBuffer::ClipToScreen(float& u0, float& v0, float& u1, float& v1)
{
	// Out of screen 
	if (u0 > w || v0 > h || u1 < 0 || v1 < 0)
		return 0;

	u0 = std::max(0.0f, u0);
	v0 = std::max(0.0f, v0);
	u1 = std::min(u1, static_cast<float>(w - 1));
	v1 = std::min(v1, static_cast<float>(h - 1));
	return 1;
}

bool FrameBuffer::IsInScreen(int u, int v)
{
	if (u < 0)
		return false;
	if (u > w - 1)
		return false;
	if (v < 0)
		return false;
	if (v > h - 1)
		return false;

	return true;
}

void FrameBuffer::Clear(unsigned bgr, float z0)
{
	SetBGR(bgr);
	// memset(zb, 0, sizeof(float) * w * h);
	for(int u = 0; u < w; ++u)
	{
		for(int v = 0; v < h; ++v)
		{
			zb[(h - 1 - v)*w + u] = 0.0f;
		}
	}
}

bool FrameBuffer::Visible(int u, int v, float curz)
{
	// First, u, v should be in screen
	if (!IsInScreen(u, v))
		return false;

	int uv = (h - 1 - v)*w + u;
	if (zb[uv] > curz)
		return false;

	zb[uv] = curz;
	return true;
}

float FrameBuffer::GetZ(int u, int v)
{
	if (!IsInScreen(u, v))
		return 0.0f;

	return zb[(h - 1 - v)*w + u];
}

unsigned FrameBuffer::Get(int u, int v)
{
	if (!IsInScreen(u, v))
		return 0xFF000000;

	return pix[(h - 1 - v)*w + u];
}

bool FrameBuffer::LoadTex(const std::string texFile)
{
	const char* fname = texFile.c_str();
	TIFF* in = TIFFOpen(fname, "r");
	if (in == nullptr)
	{
		cerr << fname << " could not be opened" << endl;
		return false;
	}

	int width, height;
	TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);
	vector<unsigned int> texMemory(width * height);

	if (TIFFReadRGBAImage(in, width, height, &texMemory[0], 0) == 0)
	{
		cerr << "failed to load " << fname << endl;
		return false;
	}
	
	// commit changes
	if (textures.find(texFile) != textures.end())
	{
		textures.erase(texFile);
	}
	textures[texFile].texture = texMemory;
	textures[texFile].w = width;
	textures[texFile].h = height;

	TIFFClose(in);
	return true;
}

void FrameBuffer::DrawSegment(V3 p0, V3 c0, V3 p1, V3 c1)
{
	V3 v2v1 = p1 - p0;
	V3 c2c1 = c1 - c0;
	int pixelN;
	if(fabsf(v2v1[0])> fabsf(v2v1[1]))
	{
		// Horizontal
		 pixelN = static_cast<int>(fabs(v2v1[0])) + 1;
	}
	else
	{
		// Vertical
		 pixelN = static_cast<int>(fabs(v2v1[1])) + 1;
	}

	for (int stepi = 0; stepi < pixelN + 1; ++stepi)
	{
		float fract = static_cast<float>(stepi) / static_cast<float>(pixelN);
		V3 point = p0 + v2v1 * fract;
		V3 color = c0 + c2c1 * fract;

		int u = static_cast<int>(point[0]);
		int v = static_cast<int>(point[1]);

		// Depth test
		if(Visible(u,v,point[2]))
			SetGuarded(u, v, color.GetColor());
	}
}

void FrameBuffer::Draw3DSegment(PPC* ppc, V3 p1, V3 c1, V3 p2, V3 c2)
{
	V3 pp1, pp2;
	
	if (!ppc->Project(p1, pp1))
		return;
	if (!ppc->Project(p2, pp2))
		return;
	DrawSegment(pp1,c1,pp2,c2);
}

void FrameBuffer::DrawRectangle(int u0, int v0, int u1, int v1, unsigned color)
{
	if (!ClipToScreen(u0, v0, u1, v1))
		return;

	for (int i = u0; i <= u1; ++i)
	{
		for (int j = v0; j <= v1; ++j)
		{
			SetGuarded(i, j, color);
		}
	}
}

void FrameBuffer::DrawCircle(int u0, int v0, int r, unsigned int color)
{
	// bounding box, then iterate the bounding box area
	int u1 = u0 - r, v1 = v0 - r, u2 = u0 + r, v2 = v0 + r;
	if (!ClipToScreen(u1, v1, u2, v2))
		return;

	for (int u = u1; u <= u2; ++u)
	{
		for (int v = v1; v <= v2; ++v)
		{
			if((u-u0)*(u - u0) + (v-v0)*(v-v0)<=r*r)
				SetGuarded(u, v, color);
		}
	}
}

void FrameBuffer::DrawEllipse(int u0, int v0, float r0, float r1, unsigned color)
{
	int u1 = u0 - r0, v1 = v0 - r1, u2 = u0 + r0, v2 = u0 + r1;
	if (!ClipToScreen(u1, v1, u2, v2))
		return;

	for (int u = u1; u <= u2; ++u)
	{
		for (int v = v1; v <= v2; ++v)
		{
			if ((u - u0)*(u - u0)/(r0*r0) + (v - v0)*(v - v0)/(r1*r1) <= 1.0f)
			{
				SetGuarded(u, v, color);
			}
		}
	}
}

void FrameBuffer::DrawPoint(int u, int v, unsigned color)
{
	SetGuarded(u, v, color);
}

void FrameBuffer::Draw3DPoint(PPC* camera, V3 p, unsigned color, int pointSize)
{
	V3 pp;
	if (!camera->Project(p, pp) || 
		pp[0] < 0 || pp[1] < 0 || 
		pp[0] >= static_cast<float>(w) || pp[1] >= static_cast<float>(h))
		return;

	int u = static_cast<int>(pp[0]);
	int v = static_cast<int>(pp[1]);
	int halfPointSize = pointSize / 2;
	DrawRectangle(u - halfPointSize, v - halfPointSize, u + halfPointSize, v + halfPointSize, color);
}

void FrameBuffer::Draw3DTriangle(PPC* camera, V3 p1, V3 p2, V3 p3, V3 color)
{
	V3 pp0, pp1, pp2;  // image space point coordinate
	if (!camera->Project(p1, pp0) || !camera->Project(p2, pp1) || !camera->Project(p3, pp2))
		return;

	AABB bbTri(pp0);
	bbTri.AddPoint(pp1);
	bbTri.AddPoint(pp2);
	ClipToScreen(bbTri.corners[0][0], bbTri.corners[0][1], bbTri.corners[1][0], bbTri.corners[1][1]);
	
	// Rasterize bbox 
	int left = static_cast<int>(bbTri.corners[0][0] + 0.5f), right = static_cast<int>(bbTri.corners[1][0] - 0.5f);
	int top = static_cast<int>(bbTri.corners[0][1] + 0.5f), bottom = static_cast<int>(bbTri.corners[1][1] - 0.5f);

	for(int i = top; i <= bottom; ++i)
	{
		for(int j = left; j <= right; ++j)
		{
			V3 p(j,i, 0.0f);
			bool s1 = InsideTriangle(p, pp0, pp1,pp2);
			bool s2 = InsideTriangle(p, pp1, pp2,pp0);
			bool s3 = InsideTriangle(p, pp2, pp0,pp1);
			if (s1 == true && s2 == true && s3 == true)
			{
				// DrawPoint(j, i, color.GetColor());
				int u = j, v = i;
				M33 m;	// Project of P based on A, B, C
				m.SetColumn(0, V3(pp0[0], pp0[1], 1.0f));
				m.SetColumn(1, V3(pp0[0], pp0[1], 1.0f));
				m.SetColumn(2, V3(pp1[0], pp1[1], 1.0f));
				V3 ABC = m.Inverse()*p;

				// Depth test
				float ppz = ABC * V3(pp0[2], pp0[2], pp1[2]);
				if (Visible(u, v, ppz))
					DrawPoint(u, v, color.GetColor());
			}
		}
	}
}

void FrameBuffer::Draw3DTriangle(PPC* ppc, V3 p0, V3 c0, V3 p1, V3 c1, V3 p2, V3 c2)
{
	V3 pp0, pp1, pp2;
	if (!ppc->Project(p0, pp0))
		return;
	if (!ppc->Project(p1, pp1))
		return;
	if (!ppc->Project(p2, pp2))
		return;

	AABB bbTri(pp0);
	bbTri.AddPoint(pp1);
	bbTri.AddPoint(pp2);
	ClipToScreen(bbTri.corners[0][0], bbTri.corners[0][1], bbTri.corners[1][0], bbTri.corners[1][1]);

	// Rasterize bbox 
	int left = static_cast<int>(bbTri.corners[0][0] + 0.5f), right = static_cast<int>(bbTri.corners[1][0] - 0.5f);
	int top = static_cast<int>(bbTri.corners[0][1] + 0.5f), bottom = static_cast<int>(bbTri.corners[1][1] - 0.5f);

	// Q matrix
	M33 abcM;
	abcM.SetColumn(0, ppc->a);
	abcM.SetColumn(1, ppc->b);
	abcM.SetColumn(2, ppc->c);
	M33 vcM;		// V1-C, V2-C, V3-C
	vcM.SetColumn(0, p0 - ppc->C);
	vcM.SetColumn(1, p1 - ppc->C);
	vcM.SetColumn(2, p2 - ppc->C);
	M33 qM = vcM.Inverse() * abcM;
	for (int i = top; i <= bottom; ++i)
	{
		for (int j = left; j <= right; ++j)
		{
			V3 pp(static_cast<float>(j) + 0.5f, static_cast<float>(i) + 0.5f, 1.0f);
			bool s1 = InsideTriangle(pp, pp0, pp1, pp2);
			bool s2 = InsideTriangle(pp, pp1, pp2, pp0);
			bool s3 = InsideTriangle(pp, pp2, pp0, pp1);
			if (s1 == true && s2 == true && s3 == true)
			{
#if defined(SCREEN_SPACE_INTERPOLATION)
				 // Screen-space interpolation
				  int u = j, v = i;
				  V3 c;
				  M33 m;	// Project of P based on A, B, C
				  m[0] = V3(pp0[0], pp0[1], 1.0f);
				  m[1] = V3(pp1[0], pp1[1], 1.0f);
				  m[2] = V3(pp2[0], pp2[1], 1.0f);
				  V3 vpr(c0[0],c1[0],c2[0]);
				  V3 vpg(c0[1],c1[1],c2[1]);
				  V3 vpb(c0[2],c1[2],c2[2]);
				  V3 vpz(pp0[2], pp1[2], pp2[2]);
				  M33 mInverse = m.Inverse();
				  V3 abcR = mInverse * vpr;
				  V3 abcG = mInverse * vpg;
				  V3 abcB = mInverse * vpb;
				  V3 abcZ = mInverse * vpz;
    
				  // Depth test
				  float ppr = abcR * pp;
				  float ppg = abcG * pp;
				  float ppb = abcB * pp;
				  float ppz = abcZ * pp;
				  c = V3(ppr, ppg, ppb);
				  if(Visible(u,v,ppz))
				  	DrawPoint(u, v, c.GetColor()); 

#elif defined(PERSPECTIVE_CORRECT_INTERPOLATION)
				// Perspective correct interpolation
				int u = j, v = i;
				float k = V3(u, v, 1.0f) * qM[1] / (qM.GetColumn(0)*V3(u, u, u) + qM.GetColumn(1)*V3(v, v, v) + qM.GetColumn(2)*V3(1.0f));
				float l = V3(u,v,1.0f) * qM[2] / (qM.GetColumn(0)*V3(u, u, u) + qM.GetColumn(1)*V3(v, v, v) + qM.GetColumn(2)*V3(1.0f));
				float w = qM.GetColumn(0)*V3(u, u, u) + qM.GetColumn(1)*V3(v, v, v) + qM.GetColumn(2)*V3(1.0f);
				if (Visible(u, v, w))
				{
					V3 c = c0 + (c1 - c0)*k + (c2 - c0)*l;
					DrawPoint(u, v, c.GetColor());
				}
#endif
			}
		}
	}
}

void FrameBuffer::Draw3DTriangleTexture(PPC* ppc, PointProperty p0, PointProperty p1, PointProperty p2, const std::string texFile)
{
	V3 pp0, pp1, pp2;
	if (!ppc->Project(p0.p, pp0))
		return;
	if (!ppc->Project(p1.p, pp1))
		return;
	if (!ppc->Project(p2.p, pp2))
		return;

	AABB bbTri(pp0);
	bbTri.AddPoint(pp1);
	bbTri.AddPoint(pp2);
	ClipToScreen(bbTri.corners[0][0], bbTri.corners[0][1], bbTri.corners[1][0], bbTri.corners[1][1]);

	M33 abcM;
	abcM.SetColumn(0, ppc->a);
	abcM.SetColumn(1, ppc->b);
	abcM.SetColumn(2, ppc->c);
	M33 vcM;
	vcM.SetColumn(0, p0.p - ppc->C);
	vcM.SetColumn(1, p1.p - ppc->C);
	vcM.SetColumn(2, p2.p - ppc->C);
	M33 qM = vcM.Inverse() * abcM;

	// Rasterize bbox
	int left = static_cast<int>(bbTri.corners[0][0] + 0.5f), right = static_cast<int>(bbTri.corners[1][0] - 0.5f);
	int top = static_cast<int>(bbTri.corners[0][1] + 0.5f), bottom = static_cast<int>(bbTri.corners[1][1] - 0.5f);

	for(int v = top; v <= bottom; ++v)
	{
		for(int u = left; u <= right; ++u)
		{
			V3 uvP(static_cast<float>(u) + 0.5f, static_cast<float>(v) + 0.5f, 1.0f);
			bool s1 = InsideTriangle(uvP, pp0, pp1, pp2);
			bool s2 = InsideTriangle(uvP, pp1, pp2, pp0);
			bool s3 = InsideTriangle(uvP, pp2, pp0, pp1);
			if(s1 == true && s2 == true && s3 == true)
			{
				float k = V3(u, v, 1.0f) * qM[1] / (qM.GetColumn(0)*V3(u, u, u) + qM.GetColumn(1)*V3(v, v, v) + qM.GetColumn(2)*V3(1.0f));
				float l = V3(u, v, 1.0f) * qM[2] / (qM.GetColumn(0)*V3(u, u, u) + qM.GetColumn(1)*V3(v, v, v) + qM.GetColumn(2)*V3(1.0f));
				float w = qM.GetColumn(0)*V3(u, u, u) + qM.GetColumn(1)*V3(v, v, v) + qM.GetColumn(2)*V3(1.0f);
				V3 st0(p0.s, p0.t, 0.0f), st1(p1.s, p1.t, 0.0f), st2(p2.s, p2.t, 0.0f);
				
				if(Visible(u,v,w))
				{
					V3 st = st0 + (st1 - st0)*k + (st2 - st0)*l;
					if(textures.find(texFile) != textures.end())
					{
						// s and t in (0.0f,1.0f)
						float s = Clamp(Fract(st[0]),0.0f,1.0f);
						float t = Clamp(Fract(st[1]),0.0f,1.0f);
						V3 color = LookupColor(texFile, s, t);
						DrawPoint(u, v, color.GetColor());
					}
				}
			}
		}
	}
}

void FrameBuffer::DrawPPC(PPC* wPPC, PPC* tPPC, float vf)
{
	float f = tPPC->GetFocal();
	float scf = vf / f;

	V3 vd = tPPC->GetVD();
	V3 tC = tPPC->C;
	V3 ta = tPPC->a * scf, tb = tPPC->b* scf, tc = tPPC->c* scf;
	float w = tPPC->w, h = tPPC->h;
	V3 c = V3(0.0f, 1.0f, 0.0f);

	// Draw a pyramid
	Draw3DSegment(wPPC, tC, c, tC + tc, c);
	// Draw3DSegment(wPPC, tC, c, tC + tc + ta * w, c);
	// Draw3DSegment(wPPC, tC, c, tC + tc + tb * h, c);
	// Draw3DSegment(wPPC, tC, c, tC + tc + ta * w + tb * h, c);
	
	Draw3DSegment(wPPC, tC + tc, c, tC + tc + ta * w, c);
	Draw3DSegment(wPPC, tC + tc, c, tC + tc + tb * h, c);
	Draw3DSegment(wPPC, tC + tc + ta * w, c, tC + tc + ta * w + tb * h, c);
	Draw3DSegment(wPPC, tC + tc + tb * h, c, tC + tc + ta * w + tb * h, c);
}

void FrameBuffer::VisualizeCurrView(PPC* ppc0, float currf, PPC* ppc1, FrameBuffer* fb1)
{
	// Iterate all the pixels drew by ppc0
	// Unproject the pixel using z buffer
	// Then draw again  by ppc1 onto the image plane
	for(int v = 0; v < h; ++v)
	{
		for(int u =0; u < w; ++u)
		{
			float z = GetZ(u, v);
			if(FloatEqual(z,0.0f))
				continue;
			V3 pP(0.5f + static_cast<float>(u), 0.5f + static_cast<float>(v), z);
			V3 pixP = ppc0->UnprojectPixel(pP[0], pP[1], currf);
			V3 cv; cv.SetColor(Get(u, v));
			fb1->Draw3DPoint(ppc1, pixP, cv.GetColor(), 1);
		}
	}
}

void FrameBuffer::VisualizeCurrView3D(PPC* ppc0, PPC* ppc1, FrameBuffer* fb1)
{
	// Iterate all the pixels drew by ppc0
	// Unproject the pixel using z buffer
	// Then draw again by ppc1 in 3D space
	for (int v = 0; v < h; ++v)
	{
		for (int u = 0; u < w; ++u)
		{
			float z = GetZ(u, v);
			if (FloatEqual(z, 0.0f))
				continue;
			V3 pP(0.5f + static_cast<float>(u), 0.5f + static_cast<float>(v), z);
			V3 pixP = ppc0->Unproject(pP);
			V3 cv; cv.SetColor(Get(u, v));
			fb1->Draw3DPoint(ppc1, pixP, cv.GetColor(), 1);
		}
	}
}

V3 FrameBuffer::LookupColor(std::string texFile, float s, float t)
{
	if(textures.find(texFile) == textures.end())
	{
		cerr << "Not found texture: " << texFile << endl;
		return 0xFF000000;
	}


	int texW = textures[texFile].w, texH = textures[texFile].h;
	float textS = s * static_cast<float>(texW-1);
	float textT = t * static_cast<float>(texH-1);

	// nearest
//	int u = int(textS);
//	int v = int(textT);
//	V3 c;
//	c.SetColor(textures[texFile].texture[(texH - 1 - v)*texW + u]);
//	return c;

	// corner case
	int u0 = max(0, static_cast<int>(textS - 0.5f)), v0 = max(0, static_cast<int>(textT - 0.5f));
	int u1 = min(texW -1, static_cast<int>(textS + 0.5f)), v1 = min(texH, static_cast<int>(textT + 0.5f));
	
	V3 c0, c1, c2, c3;
	c0.SetColor(textures[texFile].texture[(texH - 1 - v0)*texW + u0]);
	c1.SetColor(textures[texFile].texture[(texH - 1 - v0)*texW + u1]);
	c2.SetColor(textures[texFile].texture[(texH - 1 - v1)*texW + u0]);
	c3.SetColor(textures[texFile].texture[(texH - 1 - v1)*texW + u1]);

	float uf0 = static_cast<float>(u0) + 0.5f, vf0 = static_cast<float>(v0) + 0.5f;
	float intpS = Clamp(textS - uf0,0.0f,1.0f), intpT = Clamp(textT - vf0,0.0f,1.0f);
	return c0 *(1.0f - intpS)*(1.0f - intpT) + c1 *intpS * (1.0f - intpT) + c2 *(1.0f - intpS)*intpT + c3 * intpS *intpT;
}
