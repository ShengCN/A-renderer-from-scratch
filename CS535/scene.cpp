#include "scene.h"
#include "v3.h"
#include "m33.h"
#include <stdlib.h>
#include <algorithm>
#include "MathTool.h"
#include "AABB.h"
#include "TM.h"
#include <string>
#include <ctime>

Scene* scene;

#include <fstream>

#include <iostream>

Scene::Scene(): isRenderAABB(false)
{
	gui = new GUI();
	gui->show();

	int u0 = 20;
	int v0 = 20;
	int w = 640;
	int h = 480;
	//	int w = 1280;
	//	int h = 720;
	int fovf = 55.0f;
	fb = new FrameBuffer(u0, v0, w, h);
	fb->label("SW Framebuffer");
	fb->show();

	fb3 = new FrameBuffer(u0 + fb->w + 30, v0, w, h);
	fb3->label("Third Person View");
	fb3->show();

	ppc = new PPC(fb->w, fb->h, fovf);
	ppc3 = new PPC(fb3->w, fb3->h, 30.0f);

	gui->uiw->position(u0, v0 + fb->h + 60);

	// Ground Quad
	float groundSz = 1.0f, height = 0.0f;
	V3 y(0.0f, 1.0f, 0.0f), gColor(0.5f);
	V3 p0(-groundSz, -height, -groundSz), p1(-groundSz, -height, groundSz), p2(groundSz, -height, groundSz), p3(groundSz, -height, -groundSz);
	PointProperty pp0(p0, gColor, y, 0.0f, 0.0f), pp1(p1, gColor, y, 0.0f, 1.0f), pp2(p2, gColor, y, 1.0f, 1.0f), pp3(p3, gColor, y, 1.0f, 0.0f);

	TM *audi = new TM();
	TM *ground = new TM();
	TM *light = new TM();
	light->SetQuad(pp0, pp1, pp2, pp3);
	ground->SetQuad(pp0, pp1, pp2, pp3);
	audi->LoadModelBin("./geometry/teapot1K.bin");
	V3 tmC = ppc->C + ppc->GetVD() * 100.0f;
	audi->PositionAndSize(tmC, 50.0f);
	ground->PositionAndSize(tmC - y * audi->ComputeAABB().GetDiagnoalLength() * 0.2f, 200.0f);

	meshes.push_back(audi);
	meshes.push_back(ground);
	meshes.push_back(light);

	ppc->C = ppc->C + V3(0.0f, 5.0f, 0.0f);
	ppc->PositionAndOrient(ppc->C, audi->GetCenter(), V3(0.0f, 1.0f, 0.0f));
//	ppc->RevolveH(ground->GetCenter(), 40.0f);

	ppc3->C = ppc3->C + V3(330.0f, 150.0f, 300.0f);
	ppc3->PositionAndOrient(ppc3->C, audi->GetCenter(), V3(0.0f, 1.0f, 0.0f));
	
	// Lighting
	V3 L = audi->GetCenter() + V3(40.0f, 0.0f, 0.0f);
	light->PositionAndSize(L, 10.0f);

	fb->L = L;
	fb3->L = L;

	Render();
	// RenderWireFrame();
}

void Scene::Render()
{
	if (fb)
	{
		RenderTexture(ppc, fb);
	}

	// Third person view rendering
	if (fb3)
	{
		float currf = 40.0f;

		fb3->Clear(0xFFFFFFFF, 0.0f);
		fb3->DrawPPC(ppc3, ppc, currf);
		fb->VisualizeCurrView(ppc, currf, ppc3, fb3); // using a 3rd ppc to visualize current ppc
		fb->VisualizeCurrView3D(ppc, ppc3, fb3); // using a 3rd ppc to visualize current ppc
		fb3->redraw();
	}
}

void Scene::Render(PPC* currPPC, FrameBuffer* currFB)
{
	// Third person view rendering
	if (currFB)
	{
		currFB->Clear(0xFFFFFFFF, 0.0f);
		for_each(meshes.begin(), meshes.end(), [&](TM* t)
	         {
		         t->RenderFill(currPPC, currFB);
		         if (isRenderAABB)
			         t->RenderAABB(currPPC, currFB);
	         });
		currFB->redraw();
	}
}

void Scene::RenderTexture(PPC* currPPC, FrameBuffer* currFB)
{
	// Third person view rendering
	if (currFB)
	{
		currFB->Clear(0xFFFFFFFF, 0.0f);
		for_each(meshes.begin(), meshes.end(), [&](TM* t)
	         {
		         t->RenderFillTexture(currPPC, currFB);
		         if (isRenderAABB)
			         t->RenderAABB(currPPC, currFB);
	         });
		currFB->redraw();
	}
}


void Scene::RenderWireFrame()
{
	fb->Clear(0xFFFFFFFF, 0.0f);

	// Draw all triangles
	for_each(meshes.begin(), meshes.end(), [&](TM* t) { t->RenderWireFrame(ppc, fb); });

	// commit frame update
	fb->redraw();
}

Scene::~Scene()
{
	if (ppc != nullptr)
		delete ppc;
	if (ppc3 != nullptr)
		delete ppc3;
	for_each(meshes.begin(), meshes.end(), [](TM* tm) { if (tm != nullptr) tm->~TM(); });
	if (fb != nullptr)
		delete fb;
	if (fb3 != nullptr)
		delete fb3;
}

bool Scene::DBGFramebuffer()
{
	V3 p1(0.0f, 0.f, -100.0f), p2(-50.0f, 50.0f, -100.0f);
	V3 pp1(320.0f, 240.0f, 0.0f), pp2(220.0f, 140.0f, 0.0f);
	V3 c1(0.0f), c2(0.0f);
	// fb->DrawSegment(pp1, c1, pp2, c2);

	cerr << "DBGFramebuffer passed!\n";
	return true;
}

bool Scene::DBGV3()
{
	V3 v1(1.0f), v2(0.5f), v3(3.0f, 0.0f, 0.0f), v4;
	v1[0] = 0.0f;

	cerr << v1 << v1 * v2 << endl << v1 - v2 << v1.cross(v2) << v1 * 3.0f << (v1 * 3.0f) / 3.0f;
	cerr << v1.Length() << endl << v3.UnitVector();
	cerr << v1 << "x Rotate 90 degree:\n " << v1.Rotate(V3(1.0f, 0.0f, 0.0f), 90.0f);
	cerr << v1 << "y Rotate 90 degree:\n " << v1.Rotate(V3(0.0f, 1.0f, 0.0f), 90.0f);
	cerr << v1 << "z Rotate 90 degree:\n " << v1.Rotate(V3(0.0f, 0.0f, 1.0f), 90.0f);
	// cerr << "Please input a V3: " << endl;
	// cin >> v4;
	// cerr << "Your V3: "<<v4;

	v4.SetColor(0xFFFFFFFF);
	cerr << v4.GetColor() << endl;
	if (!FloatEqual(v1[0], 0.0f)
		|| !FloatEqual(v1[2], 1.0)
		|| !FloatEqual(v1 * v2, 1.0f)
		|| !FloatEqual((v1 - v2)[1], 0.5f)
		|| v1.cross(v2) != V3(0.0f, 0.5f, -0.5f)
		|| v1 * 3.0f != V3(0.0f, 3.0f, 3.0f)
		|| (v1 * 3.0f) / 3.0f != v1
		|| v3.UnitVector() != V3(1.0f, 0.0f, 0.0f)
		|| v3.UnitVector().Length() != 1.0f
		|| v1.Rotate(V3(1.0f, 0.0f, 0.0f), 90.0f) != V3(0.0f, -1.0f, 1.0f)
		|| v1.Rotate(V3(0.0f, 1.0f, 0.0f), 90.0f) != V3(1.0f, 1.0f, 0.0f)
		|| v1.Rotate(V3(0.0f, 0.0f, 1.0f), 90.0f) != V3(-1.0f, 0.0f, 1.0f))
	{
		cerr << "DBGV3 error\n";

		if (v1.Rotate(V3(1.0f, 0.0f, 0.0f), 90.0f) != V3(0.0f, -1.0f, 1.0f))
		{
			cerr << "X rotation error\n";
			cerr << "It should be: \n" << V3(0.0f, -1.0f, 1.0f) << v1.Rotate(V3(1.0f, 0.0f, 0.0f), 90.0f);
		}
		if (v1.Rotate(V3(0.0f, 1.0f, 0.0f), 90.0f) != V3(1.0f, 1.0f, 0.0f))
		{
			cerr << "Y rotation error \n";
			cerr << "It should be: \n" << V3(1.0f, 1.0f, 0.0f) << v1.Rotate(V3(0.0f, 1.0f, 0.0f), 90.0f);
		}
		if (v1.Rotate(V3(0.0f, 0.0f, 1.0f), 90.0f) != V3(-1.0f, 0.0f, 1.0f))
		{
			cerr << "Z rotation error \n";
			cerr << "It should be: \n" << V3(-1.0f, 0.0f, 1.0f) << v1.Rotate(V3(0.0f, 0.0f, 1.0f), 90.0f);
		}
		return false;
	}

	cerr << "DBGV3 passed\n";
	return true;
}

bool Scene::DBGM3()
{
	M33 m0, m1, m2, m3, m4;
	m0[0] = V3(1.0f, 0.0f, 0.0f);
	m0[1] = V3(0.0f, 1.0f, 0.0f);
	m0[2] = V3(0.0f, 0.0f, 1.0f);
	V3 v(0.5f);
	m1[0] = V3(0.5f, 0.0f, 0.0f);
	m1[1] = V3(0.0f, 0.5f, 0.0f);
	m1[2] = V3(0.0f, 0.0f, 0.5f);

	m2[0] = V3(0.0f, 0.5f, 0.0f);
	m2[1] = V3(0.0f, 0.0f, 0.1f);
	m2[2] = V3(0.0f, 0.0f, 0.0f);

	m3[0] = V3(0.0f, 0.0f, 0.0f);
	m3[1] = V3(0.5f, 0.0f, 0.0f);
	m3[2] = V3(0.0f, 0.1f, 0.0f);

	cerr << m0 << m0 * v << m0 * m1 << m0.Det() << endl;
	cerr << m0 / 2.0f << m1.Inverse() << m2 << m2.Transpose();
	// cerr << "Please input a M3: " << endl;
	// cin >> m4;
	// cerr << "Your M3: " << m4;
	if (m0[0] != V3(1.0f, 0.0f, 0.0f)
		|| m0 * v != v
		|| m0 * m1 != m1
		|| !FloatEqual(m0.Det(), 1.0f)
		|| m0 / 2.0f != m1
		|| m0.Inverse() != m0
		|| m1.Inverse() != m1 * 4.0f
		|| m2.Transpose() != m3
		|| m0.Inverse() != m0)
	{
		cerr << "DBGM3 error\n";
		return false;
	}

	cerr << "DBGM3 passed \n";
	return true;
}

bool Scene::DBGAABB()
{
	V3 p0(-1.0f, 0.0f, 0.0f), p1(5.0f, 5.0, 5.0), p2(-5.5f, 0.5f, 0.5f);
	AABB bbox(p0);
	bbox.AddPoint(p1);
	bbox.AddPoint(p2);

	if (bbox.corners[0] != V3(-5.5f, 0.0f, 0.0f) || bbox.corners[1] != p1)
	{
		cerr << "AABB not passed \n";
		return false;
	}

	cerr << "AABB passed \n";
	return true;
}

bool Scene::DBGPPC()
{
	float hfov = ppc->GetHorizontalFOV();
	cerr << "Fov: " << hfov << endl;

	if (!FloatEqual(hfov, 55.0f))
	{
		cerr << "PPC not pass!\n";
		return false;
	}

	cerr << "PPC passed \n";
	return true;
}

void Scene::DBG()
{
	// cerr << "INFO: pressed DBG" << endl;
	cerr << "Begin DBG\n";
	if (DBGV3() && DBGM3() && DBGFramebuffer() && DBGAABB() && DBGPPC()) // && DBGTM())
		cerr << "All pased! \n";
	else
		cerr << "Not pass!\n";

	Demonstration();
	fb->redraw();
}

void Scene::InitDemo()
{
	// Random axis
	TM quad;
	PointProperty p0(V3(-100.0f, 100.0f, -200.0f), V3(0.5f), V3(0.0f, 0.0f, 1.0f), 0.0f, 0.0f);
	PointProperty p1(V3(100.0f, 100.0f, -200.0f), V3(0.5f), V3(0.0f, 0.0f, 1.0f), 1.0f, 0.0f);
	PointProperty p2(V3(-100.0f, -100.0f, -200.0f), V3(0.5f), V3(0.0f, 0.0f, 1.0f), 0.0f, 1.0f);
	PointProperty p3(V3(-100.0f, 100.0f, -200.0f), V3(0.5f), V3(0.0f, 0.0f, 1.0f), 0.0f, 0.0f);
	quad.SetTriangle(p0, p1, p2);
	// meshes.push_back(quad);
}

void Scene::Demonstration()
{
	int count = 0;
	int stepN = 360;
	for (int stepi = 0; stepi < stepN; stepi++)
	{
		//string fname = "images/demo-" + to_string(count++) + ".tiff";
		char csName[50];
		sprintf_s(csName, "mydbg/test-%03d.tiff", stepi);
		string fname(csName);
		// fb->SaveAsTiff(fname.c_str());
		// meshes[2]->RotateAboutArbitraryAxis(meshes[0]->GetCenter(),V3(0.0f,1.0f,0.0f) ,1.0f);
		// fb->L = meshes[2]->GetCenter();
		ppc->RevolveH(meshes[0]->GetCenter(), 1.0f);
		Render();
		Fl::check();
	}
}