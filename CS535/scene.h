#pragma once
#include <vector>
#include "gui.h"
#include "framebuffer.h"
#include "ppc.h"
#include "TM.h"
using std::vector;

class Scene
{
public:
	GUI* gui;
	FrameBuffer* fb;
	PPC* ppc;
	PPC* wppc;	// world ppc, to track current ppc
	vector<TM*> meshes;

	Scene();
	void DBG();
	void Render();				// render all triangles in the scene
	void RenderWireFrame();

	~Scene();
private:
	bool DBGFramebuffer();
	bool DBGV3();
	bool DBGM3();
	bool DBGAABB();
	bool DBGPPC();
};

extern Scene* scene;
