#include "scene.h"
#include "v3.h"
#include "m33.h"
#include <stdlib.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <ctime>

#include "MathTool.h"
#include "AABB.h"
#include "TM.h"
#include "GlobalVariables.h"

Scene* scene;
int TM::tmIDCounter = 0;

Scene::Scene(): isRenderAABB(false)
{
	gui = new GUI();
	gui->show();
	auto gv = GlobalVariables::Instance();
	gv->curScene = this;

	int u0 = 20;
	int v0 = 20;
	int w = gv->isHighResolution? gv->highResoW: gv->resoW;
	int h = gv->isHighResolution ? gv->highResoH : gv->resoH;
	//	int w = 1280;
	//	int h = 720;
	int fovf = 70.0f;
	fb = new FrameBuffer(u0, v0, w, h);
	fb->label("SW Framebuffer");
	fb->show();

	fb3 = new FrameBuffer(u0 + fb->w + 30, v0, w, h);
	fb3->label("Third Person View");
	fb3->show();

	ppc = new PPC(fb->w, fb->h, fovf);
	ppc3 = new PPC(fb3->w, fb3->h, 30.0f);

	gui->uiw->position(u0, v0 + fb->h + 60);

	InitDemo();
	
	Render();
}

void Scene::Render()
{
	if(GlobalVariables::Instance()->isShadow)
	{
		UpdateSM();
	}

	if (fb)
	{
		Render(ppc, fb);
	}

	// Third person view rendering
	if (fb3)
	{
		float currf = 40.0f;

		fb3->ClearBGR(0xFFFFFFFF, 0.0f);
		fb3->DrawPPC(ppc3, ppc, currf);
		fb->VisualizeCurrView(ppc, currf, ppc3, fb3); // using a 3rd ppc to visualize current ppc
		fb->VisualizeCurrView3D(ppc, ppc3, fb3); // using a 3rd ppc to visualize current ppc
		
		// Visualize lights
		for(auto l:lightPPCs)
		{
			fb3->Draw3DPoint(ppc3, l->C, 0xFFFFFF00, 10);
		}

		// Visualize bbs
		for (auto r : refletors)
		{
			for (auto rb : r->reflectorBB)
			{
				rb->RenderBB(ppc3,fb3);
			}
		}
		fb3->redraw();
	}
}

void Scene::Render(PPC* currPPC, FrameBuffer* currFB)
{
	if (currFB)
	{
		currFB->ClearBGR(0xFF999999, 0.0f);
		currFB->DrawCubeMap(currPPC, cubemap.get());

		for (auto t : meshes)
		{
			t->RenderFill(currPPC, currFB);
			if (isRenderAABB)
				t->RenderAABB(currPPC, currFB);
		}

		for (auto r : refletors)
		{
			r->RenderFill(currPPC, currFB);

			if (isRenderAABB)
				r->RenderAABB(currPPC, currFB);
		}

		currFB->redraw();
	}
}

void Scene::RenderWireFrame()
{
	fb->ClearBGR(0xFFFFFFFF, 0.0f);

	// Draw all triangles
	for_each(meshes.begin(), meshes.end(), [&](TM* t) { t->RenderWireFrame(ppc, fb); });

	// commit frame update
	fb->redraw();
}

void Scene::RenderZbuffer(PPC* currPPC, FrameBuffer* currFB)
{
	if (currFB)
	{
		currFB->ClearZ(0.0f);

		if (GlobalVariables::Instance()->debugZbuffer)
			currFB->ClearBGR(0xFF000000, 0.0f);

		// Draw all triangles
		for (auto m : meshes)
		{
			m->RenderFillZ(currPPC, currFB);
		}

		// commit frame update
		currFB->redraw();
	}
}

void Scene::UpdateSM()
{
	for (size_t li = 0; li < lightPPCs.size(); ++li)
	{
		RenderZbuffer(lightPPCs[li].get(), shadowMaps[li].get());
	}
}

void Scene::RenderBB()
{
	for (auto r : refletors)
	{
		auto tmId = r->id;

		// Pre-compute some shared variables
		auto rCenter = r->GetCenter();
		auto rSize = r->ComputeAABB().GetDiagnoalLength();

		// Update all bb
		r->reflectorBB.clear();
		r->reflectorBB.reserve(refletors.size() - 1);

		// for each other object
		for (auto otherTM : refletors)
		{
			if (otherTM->id == tmId)
				continue;

			// prepare bb, currPPC, currFB
			auto otherTmCenter = otherTM->GetCenter();
			shared_ptr<BillBoard> bb = make_shared<BillBoard>();
			V3 n = (rCenter - otherTmCenter).UnitVector();
			
			// Assump up is y axis
			bb->SetBillboard(rCenter, n, V3(0.0f, 1.0f, 0.0f), rSize);

			int w = GlobalVariables::Instance()->resoW;
			int h = GlobalVariables::Instance()->resoH;
			float fovf = 55.0f;
			shared_ptr<PPC> ppc = make_shared<PPC>(w, h, fovf);
			shared_ptr<FrameBuffer> bbFB = make_shared<FrameBuffer>(0, 0, w, h);
			ppc->PositionAndOrient(rCenter, otherTmCenter, V3(0.0f, 1.0f, 0.0f));

			// render it into the currFB
			RenderBB(ppc.get(), bbFB.get(), otherTM.get());

			// Save the result 
			bb->fbTexture = bbFB;
			bb->mesh->PositionAndSize(otherTmCenter, rSize);

			// commit the new billboard for the reflector
			r->reflectorBB.push_back(bb);
		}
	}
}

// Only render the target
void Scene::RenderBB(PPC* currPPC, FrameBuffer* currFB, TM *reflector)
{
	if (currPPC && currFB && reflector)
	{
		currFB->ClearBGR(0x00999999, 0.0f);
		reflector->RenderFill(currPPC, currFB);
		currFB->redraw();
	}
}

V3 Scene::GetSceneCenter()
{
	V3 ret(0.0f);
	for (auto m : meshes)
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

void Scene::DBGZBuffer(string outFile, FrameBuffer* curfb)
{
	ofstream out(outFile);
	for (int u = 0; u < curfb->w; ++u)
	{
		for (int v = 0; v < curfb->h; ++v)
		{
			out << curfb->GetZ(u, v) << "\t";
		}
		out << endl;
	}
	out.close();
}

void Scene::InitializeLights()
{
	// Prepare for visualize the light
	V3 y(0.0f, 1.0f, 0.0f), gColor(0.5f);
	float lightsz = 1.0f, height = 0.0f;

	V3 L0 = meshes[0]->GetCenter() + V3(0.0f, 50.0f, 50.0f);

	// Shadow maps
	int u0 = 20, v0 = 20;
	float fovf = 55.0f;
	int w = 640;
	int h = 480;
	shared_ptr<PPC> l0PPC = make_shared<PPC>(w, h, fovf);
	shared_ptr<FrameBuffer> l0SM = make_shared<FrameBuffer>(u0 + fb->w * 2 + 30, v0, w, h);
	l0SM->label("Light 1 Shadows");

	l0SM->ClearBGR(0xFFFFFFFF, 0.0f);
	l0PPC->PositionAndOrient(L0, meshes[0]->GetCenter(), V3(0.0f, 1.0f, 0.0f));

	// commit to framebuffer
	shadowMaps.push_back(l0SM);
	lightPPCs.push_back(l0PPC);
	UpdateSM();
}

void Scene::InitDemo()
{
	cubemap = make_shared<CubeMap>();
	
	// prepare cubemap
	string folder = "images/cubemaps/";
	std::vector<string> fnames{
		folder + "top.tiff",
		folder + "front.tiff",
		folder + "ground.tiff",
		folder + "back.tiff",
		folder + "right.tiff",
		folder + "left.tiff" };
	float fovf = 90.0f;
	std::vector<shared_ptr<PPC>> cubemapPPCs;
	cubemapPPCs.push_back(make_shared<PPC>(480, 480, fovf));
	cubemapPPCs.push_back(make_shared<PPC>(480, 480, fovf));
	cubemapPPCs.push_back(make_shared<PPC>(480, 480, fovf));
	cubemapPPCs.push_back(make_shared<PPC>(480, 480, fovf));
	cubemapPPCs.push_back(make_shared<PPC>(480, 480, fovf));
	cubemapPPCs.push_back(make_shared<PPC>(480, 480, fovf));
	cubemapPPCs[0]->PositionAndOrient(V3(0.0f), V3(0.0f, 1.0f, 0.0f), V3(0.0f, 0.0f, 1.0f));
	cubemapPPCs[1]->PositionAndOrient(V3(0.0f), V3(0.0f, 0.0f, -1.0f), V3(0.0f, 1.0f, 0.0f));
	cubemapPPCs[2]->PositionAndOrient(V3(0.0f), V3(0.0f, -1.0f, 0.0f), V3(0.0f, 0.0f, -1.0f));
	cubemapPPCs[3]->PositionAndOrient(V3(0.0f), V3(0.0f, 0.0f, 1.0f), V3(0.0f, 1.0f, 0.0f));
	cubemapPPCs[4]->PositionAndOrient(V3(0.0f), V3(-1.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f));
	cubemapPPCs[5]->PositionAndOrient(V3(0.0f), V3(1.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f));
	cubemap->LoadCubeMap(fnames, cubemapPPCs);

	// Init objects
	TM* ground = new TM();
	auto teapot = make_shared<TM>();
	auto teapot1 = make_shared<TM>();
	auto teapot2 = make_shared<TM>();
	shared_ptr<BillBoard> billboard = make_shared<BillBoard>();
	teapot->LoadModelBin("geometry/teapot1K.bin");
	teapot->isEnvMapping = true;
	teapot->isShowObjColor = false;
	teapot1->LoadModelBin("geometry/teapot1K.bin");
	teapot1->isEnvMapping = true;
	teapot1->isShowObjColor = true;
	teapot1->SetAllPointsColor(V3(0.0f, 0.0f, 0.7f));

	teapot2->LoadModelBin("geometry/teapot1K.bin");
	teapot2->isEnvMapping = true;
	teapot2->isShowObjColor = false;

	ground->isEnvMapping = false;
	ground->isShowObjColor = true;

	ground->SetQuad(V3(0.0f), V3(0.0f, 1.0f, 0.0f), V3(0.0f, 0.0f, -1.0f), 1.0f, 1.0f, 1.0f);
	billboard->SetBillboard(V3(0.0f), V3(0.0f, 1.0f, 0.0f), V3(0.0f,0.0f,-1.0f) , 1.0f, 1.0f,1.0f);

	float obsz = 50.0f;
	V3 tmC = ppc->C + ppc->GetVD() * 100.0f;
	teapot->PositionAndSize(V3(0.0f), obsz);
	teapot1->PositionAndSize(V3(obsz * 0.75f,  0.0f, -obsz * 0.75f), obsz);
	teapot2->PositionAndSize(V3(-obsz * 0.75f, 0.0f, -obsz * 0.75f), obsz);

	// Textures
	string checkerBoxTexName = GlobalVariables::Instance()->checkerBoxTexName;
	fb->LoadTexture(checkerBoxTexName);

	ground->PositionAndSize(teapot->GetCenter() + V3(0.0f, -obsz * 0.3f, 0.0f), obsz * 2.0f);
	ground->SetText(checkerBoxTexName);
	billboard->mesh->PositionAndSize(teapot->GetCenter() + V3(0.0f, -obsz * 0.3f, 0.0f), obsz * 2.0f);
	billboard->mesh->SetText(checkerBoxTexName);

	meshes.push_back(ground);
	refletors.push_back(teapot);
	refletors.push_back(teapot1);
	refletors.push_back(teapot2);
	sceneBillboard.push_back(billboard);

	ppc->C = ppc->C - tmC + V3(0.0f, 15.0f, 0.0f);
	ppc->PositionAndOrient(ppc->C, refletors[GlobalVariables::Instance()->tmAnimationID]->GetCenter(), V3(0.0f, 1.0f, 0.0f));

	ppc3->C = ppc3->C + V3(330.0f, 150.0f, 300.0f);
	ppc3->PositionAndOrient(ppc3->C, refletors[GlobalVariables::Instance()->tmAnimationID]->GetCenter(), V3(0.0f, 1.0f, 0.0f));

	InitializeLights();

	// Prepare BB
	RenderBB();
}

void Scene::Demonstration()
{
	// Morphing 
	auto teapotC = refletors[GlobalVariables::Instance()->tmAnimationID]->GetCenter();
	int stepN = 360;
	for(int stepi = 0; stepi < stepN; ++stepi)
	{
		float fract = static_cast<float>(stepi) / static_cast<float>(stepN - 1);
		refletors[GlobalVariables::Instance()->tmAnimationID]->SphereMorph(teapotC, 13.0f, fract);
		 refletors[GlobalVariables::Instance()->tmAnimationID]->WaterAnimation(stepi);
		
		// ppc->RevolveH(meshes[0]->GetCenter(), 1.0f);
		// sceneBillboard[0]->mesh->Translate(V3(0.0f, 0.0f, -1.0f));
		
		Render();
		Fl::check();

		if(GlobalVariables::Instance()->isRecording)
		{
			char buffer[50];
			sprintf_s(buffer, "images/reflect-%03d.tiff", stepi);
			fb->SaveAsTiff(buffer);
		}
	}
}
