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
	for (int uv = 0; uv < w * h; uv++)
		pix[uv] = bgr;
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

void FrameBuffer::SetGuarded(int u, int v, unsigned int color)
{
	// clip to window 
	if (!IsInScreen(u, v))
		return;

	Set(u, v, color);
}

// load a tiff image to pixel buffer
void FrameBuffer::LoadTiff(char* fname)
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
void FrameBuffer::SaveAsTiff(char* fname)
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
	for (int i = 0; i < w * h; ++i)
		zb[i] = z0;
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

	int u = static_cast<int>(p0[0]);
	int v = static_cast<int>(p0[1]);

	for (int stepi = 0; stepi < pixelN + 1; ++stepi)
	{
		float fract = static_cast<float>(stepi) / static_cast<float>(pixelN);
		V3 point = p0 + v2v1 * fract;
		V3 color = c0 + c2c1 * fract;

		int u = static_cast<int>(point[0]);
		int v = static_cast<int>(point[1]);

		// Depth test
		if(!Visible(u,v,point[2]))
			continue;

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

	for (int i = u0; i < u1; ++i)
	{
		for (int j = v0; j < v1; ++j)
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

	for (int i = top; i <= bottom; ++i)
	{
		for (int j = left; j <= right; ++j)
		{
			V3 p(j, i, 1.0f);
			bool s1 = InsideTriangle(p, pp0, pp1, pp2);
			bool s2 = InsideTriangle(p, pp1, pp2, pp0);
			bool s3 = InsideTriangle(p, pp2, pp0, pp1);
			if (s1 == true && s2 == true && s3 == true)
			{
				int u = j, v = i;
				V3 c;
				M33 m;	// Project of P based on A, B, C
				m.SetColumn(0, V3(pp0[0], pp0[1], 1.0f));
				m.SetColumn(1, V3(pp1[0], pp1[1], 1.0f));
				m.SetColumn(2, V3(pp2[0], pp2[1], 1.0f));
				V3 ABC = m.Inverse()*p;
				M33 cm;	// color matrix
				cm.SetColumn(0, c0);
				cm.SetColumn(1, c1);
				cm.SetColumn(2, c2);
				c = cm * ABC;

				// Depth test
				float ppz = ABC * V3(pp0[2], pp1[2], pp2[2]);
				if(Visible(u,v,ppz))
					DrawPoint(u, v, c.GetColor());
			}
		}
	}
}

void FrameBuffer::DrawPPC(PPC* wPPC, PPC* tPPC)
{
	float f = tPPC->GetF();

	V3 vd = tPPC->GetVD();
	V3 tC = tPPC->C;
	V3 ta = tPPC->a, tb = tPPC->b, tc = tPPC->c;
	float w = tPPC->w, h = tPPC->h;
	V3 c = V3(0.0f, 1.0f, 0.0f);

	// Draw a pyramid
	Draw3DSegment(wPPC, tC, c, tC + tc, c);
	Draw3DSegment(wPPC, tC, c, tC + tc + ta * w, c);
	Draw3DSegment(wPPC, tC, c, tC + tc + tb * h, c);
	Draw3DSegment(wPPC, tC, c, tC + tc + ta * w + tb * h, c);
	
	Draw3DSegment(wPPC, tC + tc, c, tC + tc + ta * w, c);
	Draw3DSegment(wPPC, tC + tc, c, tC + tc + tb * h, c);
	Draw3DSegment(wPPC, tC + tc + ta * w, c, tC + tc + ta * w + tb * h, c);
	Draw3DSegment(wPPC, tC + tc + tb * h, c, tC + tc + ta * w + tb * h, c);
}
