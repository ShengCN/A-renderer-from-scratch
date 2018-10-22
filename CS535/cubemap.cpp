#include "CubeMap.h"


void CubeMap::LoadCubeMap(vector<const string> fnames)
{
	if(!cubemapFB)
		cubemapFB = make_shared<FrameBuffer>(0,0,0,0);

	cubemapFB->textures.clear();
	for(auto f:fnames)
	{
		cubemapFB->LoadTex(f);
	}
}

V3 CubeMap::LookupColor(V3 dir)
{
	V3 color(0.0f);

	return color;
}

CubeMap::CubeMap():_lastFB(0)
{
}


CubeMap::~CubeMap()
{
}
