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
	int r = 30;
	int stepN = 100;

	for (int stepi = 0; stepi < stepN; ++stepi)
	{
		fb->SetBGR(0xFFFFFFFF);
		fb->DrawRectangle(u0 + stepi, v0, u1+stepi, v1, 0xFF00FFFF);
		fb->DrawCircle(u0+stepi, v0, r, 0xFFFF0000);
		fb->redraw();
		Fl::check();
	}

	std::cerr << "DBGFramebuffer passed!\n";
	return true;
}

bool Scene::DBGV3()
{
	auto FloatEqual = [&](float a, float b)
	{
		return std::abs(a - b) < 1e-7;
	};

	V3 v1(1.0f), v2(0.5f);
	v1[0] = 0.0f;
	if (!FloatEqual(v1[0], 0.0f)
		|| !FloatEqual(v1[2], 1.0)
		|| !FloatEqual(v1*v2, 1.0f)
		|| !FloatEqual((v1-v2)[1],0.5f))
	{
		cerr << "DBGV3 error\n";
		return false;
	}

	cerr << v1 << v1 * v2 << endl << v1 - v2;
	cerr << "DBGV3 passed\n";
	return true;
}

bool Scene::DBGM3()
{
	M33 m0,m1;
	m0[0] = V3(1.0f, 0.0f, 0.0f);
	m0[1] = V3(0.0f, 1.0f, 0.0f);
	m0[2] = V3(0.0f, 0.0f, 1.0f);
	V3 v(0.5f);
	m1[0] = V3(0.5f, 0.0f, 0.0f);
	m1[1] = V3(0.0f, 0.5f, 0.0f);
	m1[2] = V3(0.0f, 0.0f, 0.5f);


	if(m0[0] != V3(1.0f, 0.0f, 0.0f) 
		|| m0*v !=v
		|| m0*m1 != m1)
	{
		cerr << "DBGM3 error\n";
	}

	cerr << m0 << m0 * v << m0 * m1;
	cerr << "DBGM3 passed \n";
	return true;
}

void Scene::DBG() {
	
	// cerr << "INFO: pressed DBG" << endl;
	cerr << "Begin DBG\n";
	if (DBGV3() && DBGM3() && DBGFramebuffer())
		cerr << "All pased! \n";
	else
		cerr << "Not pass!\n";
	
	fb->redraw();
}
