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
#include "GlobalVariables.h"

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
//	fb3->show();

	ppc = new PPC(fb->w, fb->h, fovf);
	ppc3 = new PPC(fb3->w, fb3->h, 30.0f);

	gui->uiw->position(u0, v0 + fb->h + 60);

	// Ground Quad
	TM *auditorium = new TM();
	TM *ground = new TM();
	auditorium->LoadModelBin("./geometry/bunny.bin");
	float groundsz = 1.0f;
	V3 gColor(0.9f), y(0.0f,1.0f,0.0f);
	V3 p0(-groundsz, 0.0f, -groundsz), p1(-groundsz, 0.0f, groundsz), p2(groundsz, 0.0f, groundsz), p3(groundsz, 0.0f, -groundsz);
	PointProperty pp0(p0, gColor, y, 0.0f, 0.0f), pp1(p1, gColor, y, 0.0f, 1.0f), pp2(p2, gColor, y, 1.0f, 1.0f), pp3(p3, gColor, y, 1.0f, 0.0f);
	ground->SetQuad(pp0, pp1, pp2, pp3);

	float obsz = 30.0f;
	V3 tmC = ppc->C + ppc->GetVD() * 100.0f;
	auditorium->PositionAndSize(tmC, obsz);
	ground->PositionAndSize(tmC - y * (auditorium->ComputeAABB().GetDiagnolVector() * y) * 0.5f, 50.0f);
	meshes.push_back(auditorium);
	meshes.push_back(ground);

	ppc->C = ppc->C + V3(0.0f, 10.0f, 0.0f);
	ppc->PositionAndOrient(ppc->C, auditorium->GetCenter(), V3(0.0f, 1.0f, 0.0f));
//	ppc->RevolveH(ground->GetCenter(), 40.0f);

	ppc3->C = ppc3->C + V3(330.0f, 150.0f, 300.0f);
	ppc3->PositionAndOrient(ppc3->C, auditorium->GetCenter(), V3(0.0f, 1.0f, 0.0f));
	
	// Lighting
	InitializeLights();

	// Commit some global variables
	auto gv = GlobalVariables::Instance();
	gv->curScene = this;
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
	if (currFB)
	{
		currFB->Clear(0xFF999999, 0.0f);
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

V3 Scene::GetSceneCenter()
{
	V3 ret(0.0f);
	for(auto m:meshes)
	{
		ret = ret + m->GetCenter();
	}
	ret = ret / static_cast<float>(meshes.size());
	return ret;
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

void Scene::InitializeLights()
{
	// Prepare for visualize the light
	V3 y(0.0f, 1.0f, 0.0f), gColor(0.5f);
	float lightsz = 1.0f, height = 0.0f;
	V3 p0(-lightsz, -height, -lightsz), p1(-lightsz, -height, lightsz), p2(lightsz, -height, lightsz), p3(lightsz, -height, -lightsz);
	PointProperty pp0(p0, gColor, y, 0.0f, 0.0f), pp1(p1, gColor, y, 0.0f, 1.0f), pp2(p2, gColor, y, 1.0f, 1.0f), pp3(p3, gColor, y, 1.0f, 0.0f);
	TM *lightms0 = new TM();
	TM *lightms1 = new TM();
	lightms0->SetQuad(pp0, pp1, pp2, pp3);
	lightms1->SetQuad(pp0, pp1, pp2, pp3);
	meshes.push_back(lightms0);
	meshes.push_back(lightms1);

	V3 L0 = meshes[0]->GetCenter() + V3(40.0f, 0.0f, 0.0f);
	V3 L1 = meshes[0]->GetCenter() + V3(0.0f, 0.0f, 40.0f);
	lightms0->PositionAndSize(L0, 10.0f);
	lightms1->PositionAndSize(L1, 10.0f);

	// Shadow maps
	int u0 = 20, v0 = 20, sz = 480;
	float fovf = 55.0f;
	shared_ptr<PPC> l0PPC = make_shared<PPC>(sz, sz, fovf);
	shared_ptr<PPC> l1PPC = make_shared<PPC>(sz, sz, fovf);
	shared_ptr<FrameBuffer> l1SM = make_shared<FrameBuffer>(u0 + fb->w + 30, v0, sz, sz);
	shared_ptr<FrameBuffer> l2SM = make_shared<FrameBuffer>(u0 + fb->w * 2, v0, sz, sz);
	l1SM->label("Light 1 Shadows");
	l1SM->show();
	l2SM->label("Light 2 Shadows");
	l2SM->show();
	l1SM->Clear(0xFFFFFFFF, 0.0f);
	l2SM->Clear(0xFFFFFFFF, 0.0f);
	l0PPC->PositionAndOrient(L0, meshes[0]->GetCenter(), V3(0.0f,1.0f,0.0f));
	l1PPC->PositionAndOrient(L1, meshes[0]->GetCenter(), V3(0.0f, 1.0f, 0.0f));
	
	// Render Shadow map
	RenderTexture(l0PPC.get(), l1SM.get());
	RenderTexture(l1PPC.get(), l2SM.get());

	// commit to framebuffer
	shadowMaps.push_back(l1SM);
	shadowMaps.push_back(l2SM);
	lightPPCs.push_back(l0PPC);
	lightPPCs.push_back(l1PPC);
	fb->Ls.push_back(L0);
	fb->Ls.push_back(L1);
	fb3->Ls.push_back(L0);
	fb3->Ls.push_back(L1);
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
