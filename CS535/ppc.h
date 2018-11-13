#pragma once
#include "v3.h"

class PPC
{
public:
	V3 a, b, c, C;	
	int w, h;

	PPC(int _w, int _h, float hfov);
	void Translate(V3 v);
	int Project(V3 P, V3 &ProjP);
	bool IsInSideImagePlane(V3 pp);

	// Access
	V3 GetVD();							// view direction
	float GetFocal();					// focal length
	float GetHorizontalFOV();
	float GetVerticalFOV();
	// V3 GetPrincipalPoint();			// pixel coordinates of COP project onto image plane
	V3 GetRay(int u, int v);			// Get Ray(vector) for pixel(u,v)
	V3 GetRayCenter(int u, int v);		// Get Ray pixel center(point)
	V3 Unproject(V3 pp);				// unproject pixel point
	V3 UnprojectPixel(float uf, float vf, float currf);	// unproject pixel to image plane

	// Navigation
	void Pan(float theta);				// pan roataion
	void Tilt(float theta);		
	void Roll(float theta);		
	void RevolveH(V3 p, float theta);	// Revolve Horizontally
	void RevolveV(V3 p, float theta);	// Revolve Vertically
	
	// Position
	void PositionAndOrient(V3 newC, V3 lap, V3 up);		// lap->look at point
	void Zoom(float theta);		// zoom in or out, change of focal length
	// void ChangeResolution(int _w, int _h);

	// View Interpolations
	void SetInterpolated(PPC *ppc0, PPC *ppc1, float fract);
	void SaveBin(std::string fname);
	void LoadBin(std::string fname);

	// Keyboard handles
	void MoveForward(float delta);
	void MoveLeft(float delta);
	void MoveDown(float delta);

	void SetIntrinsicsHW();
	void SetExtrinsicsHW();
};

