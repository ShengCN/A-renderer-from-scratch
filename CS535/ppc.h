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
};

