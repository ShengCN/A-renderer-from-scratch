#include "scene.h"
#include "v3.h"
#include "m33.h"
#include <stdlib.h>

Scene *scene;

using namespace std;

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

void Scene::Render() {

	fb->SetBGR(0xFFFFFFFF);
	fb->redraw();

}

void Scene::DBG() {

	cerr << "INFO: pressed DBG" << endl;

}