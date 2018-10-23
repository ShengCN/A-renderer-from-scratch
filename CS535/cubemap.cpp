#include "CubeMap.h"
#include <fstream>

void CubeMap::LoadCubeMap(vector<string> fnames, vector<shared_ptr<PPC>> _ppcs)
{
	if(!cubemapFB)
		cubemapFB = make_shared<FrameBuffer>(0,0,0,0);

	bool isSuccess = true;
	ppcs = _ppcs;
	cubemapFB->textures.clear();
	mapOrder.clear();
	for(auto f:fnames)
	{
		if (!cubemapFB->LoadTex(f)) isSuccess = false;
		mapOrder.push_back(f);
	}

	if (isSuccess)
		cerr << "Succesfully load cube maps! \n";
}

V3 CubeMap::LookupColor(V3 dir)
{
	V3 color(0.0f);
	if (ppcs.size() != 6)
		return color;

	int count = 0;
	dir = dir.UnitVector();
	V3 p = dir, pp(0.0f);

	while(count++ < 6)
	{
		auto curPPC = ppcs[_lastFB];
		curPPC->Project(p, pp);
		if (curPPC->IsInSideImagePlane(pp))
		{
			// catched by one ppc
			float s = pp[0] / static_cast<float>(curPPC->w), t = pp[1] / static_cast<float>(curPPC->h);
			auto tex = cubemapFB->textures[mapOrder[_lastFB]].back();
			float alpha = 0.0f;
			color = cubemapFB->BilinearLookupColor(tex, s, t, alpha);
			return color;
		}

		_lastFB = (_lastFB + 1) % 6;
	}

	return color;
}

CubeMap::CubeMap():_lastFB(0)
{
}


CubeMap::~CubeMap()
{
}
