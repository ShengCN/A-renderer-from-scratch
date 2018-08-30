#pragma once

#include "gui.h"
#include "framebuffer.h"
#include <vector>
#include <functional>

class Scene
{
public:


	GUI* gui;
	FrameBuffer* fb;
	Scene();
	void DBG();
	void Render();
	
private:
	bool DBGFramebuffer();
	bool DBGV3();
	bool DBGM3();
};

extern Scene* scene;
