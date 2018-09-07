#pragma once

#include "gui.h"
#include "framebuffer.h"
#include "ppc.h"

class Scene
{
public:


	GUI* gui;
	FrameBuffer* fb;
	PPC* ppc;
	Scene();
	void DBG();
	void Render();
	
private:
	bool DBGFramebuffer();
	bool DBGV3();
	bool DBGM3();
	bool DBGAABB();
};

extern Scene* scene;
