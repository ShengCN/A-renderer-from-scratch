#pragma once
#include "framebuffer.h"

class CubeMap
{
private:
	int _lastFB;

public:
	shared_ptr<FrameBuffer> cubemapFB;
	shared_ptr<PPC> ppcs;

	void LoadCubeMap(vector<const string> fnames);
	V3 LookupColor(V3 dir);
	CubeMap();
	~CubeMap();
};

