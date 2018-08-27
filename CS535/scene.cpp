#include "scene.h"
#include "v3.h"
#include "m33.h"
#include <stdlib.h>
#include <algorithm>

Scene *scene;

#include <fstream>

#include <iostream>

Scene::Scene() {

	gui = new GUI();
	gui->show();

	int u0 = 20;
	int v0 = 20;
	int w = 640;
	int h = 480;

	fb = new FrameBuffer(u0, v0, w, h);
	fb->label("SW Framebuffer");
	fb->show();


	gui->uiw->position(u0, v0 + fb->h + 60);

	Render();

}

void Scene::Render() 
{
	fb->SetBGR(0xFFFFFFFF);
	fb->redraw();
}

bool Scene::DBGFramebuffer()
{
	int u0 = 50, v0 = 50, u1 = 300, v1 = 300;
	fb->DrawRectangle(u0, v0, u1, v1,0xFF00FFFF);
	int r = 30;
	fb->DrawCircle(u0, v0, r, 0xFFFF0000);
	std::cerr << "DBGFramebuffer passed!\n";
	return true;
}

bool Scene::DBGV3()
{
	return true;
}

bool Scene::DBGM3()
{
	return true;
}

void Scene::DBG() {
	
	// cerr << "INFO: pressed DBG" << endl;
	cerr << "Begin DBG\n";
	if (DBGFramebuffer() && DBGV3() && DBGM3())
		cerr << "All pased! \n";
	else
		cerr << "Not pass!\n";
	
	fb->redraw();
}
