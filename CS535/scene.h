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
	PPC* wppc;	// world ppc, to track current ppc
	Scene();
	void DBG();
	void Render();

private:
	bool DBGFramebuffer();
	bool DBGV3();
	bool DBGM3();
	bool DBGAABB();
	bool DBGPPC();
	bool DBGTM();
};

extern Scene* scene;
