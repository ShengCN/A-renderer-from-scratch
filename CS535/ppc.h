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

	// Access
	V3 GetVD();					// view direction
	float GetF();				// focal length
	float GetHorizontalFOV();
	float GetVerticalFOV();
	// V3 GetPrincipalPoint();	// pixel coordinates of COP project onto image plane
	V3 GetRay(int u, int v);	// Get Ray(vector) for pixel(u,v)
	V3 GetRayCenter(int u, int v);    // Get Ray pixel center(point)

	// Navigation
	void Pan(float theta);		// pan roataion
	void Tilt(float theta);		
	void Roll(float theta);		
	void RevolveH(V3 p, float theta);	// Revolve Horizontally
	void RevolveV(V3 p, float theta);	// Revolve Vertically
	
	// Position
	void PositionAndOrient(V3 newC, V3 lap, V3 up);		// look at a point, distance d, up vector up

	// Internal parameters change
	void Zoom(float theta);		// zoom in or out
	void ChangeResolution(int _w, int _h);

	// View Interpolations
	void SetInterpolated(PPC *ppc0, PPC *ppc1, float fract);
};

