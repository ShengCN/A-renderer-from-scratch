#include "scene.h"
#include "v3.h"
#include "m33.h"
#include <stdlib.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <random>
#include <omp.h>

#include "MathTool.h"
#include "AABB.h"
#include "TM.h"
#include "GlobalVariables.h"
#include "ray.h"
#include <future>
#include "hitable.h"
#include "sphere.h"
#include "hitable_list.h"
#include "material.h"

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
	int w = gv->isHighResolution ? gv->highResoW : gv->resoW;
	int h = gv->isHighResolution ? gv->highResoH : gv->resoH;

	int fovf = 70.0f;
	fb = new FrameBuffer(u0, v0, w, h);
	fb->label("SW Framebuffer");
	fb->show();

	hwfb = new FrameBuffer(u0 + fb->w + 30, v0, w, h);
	hwfb->label("HW Framebuffer");
	hwfb->ishw = true;
	hwfb->show();

	fb3 = new FrameBuffer(u0 + fb->w + 30, v0, w, h);
	fb3->label("Third Person View");
	// fb3->show();

	ppc = new PPC(fb->w, fb->h, fovf);
	ppc3 = new PPC(fb3->w, fb3->h, 55.0f);

	gui->uiw->position(u0, v0 + fb->h + 60);

	InitDemo();
	Render();
}

void Scene::Render()
{
	if (GlobalVariables::Instance()->isShadow)
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

		fb3->ClearBGRZ(0xFFFFFFFF, 0.0f);
		fb3->DrawPPC(ppc3, ppc, currf);
		fb->VisualizeCurrView(ppc, currf, ppc3, fb3); // using a 3rd ppc to visualize current ppc
		fb->VisualizeCurrView3D(ppc, ppc3, fb3); // using a 3rd ppc to visualize current ppc

		// Visualize lights
		for (auto l : lightPPCs)
		{
			fb3->Draw3DPoint(ppc3, l->C, 0xFFFFFF00, 10);
		}

		// Visualize bbs
		for (auto r : refletors)
		{
			for (auto rb : r->reflectorBB)
			{
				rb->RenderBB(ppc3, fb3);
			}
		}
		fb3->redraw();
	}
}

void Scene::Render(PPC* currPPC, FrameBuffer* currFB)
{
	if (currFB)
	{
		currFB->ClearBGRZ(0xFF999999, 0.0f);

		if (!GlobalVariables::Instance()->isRayTracing)
		{
			if(cubemap)
				currFB->DrawCubeMap(currPPC, cubemap.get());
			
			for (auto t : meshes)
			{
				BeginCountingTime();
				t->RenderFill(currPPC, currFB);
				PrintTime("SW render "+ to_string(t->id) + string(" spends: "));
				if (isRenderAABB)
					t->RenderAABB(currPPC, currFB);
			}

			for (auto r : refletors)
			{
				BeginCountingTime();
				r->RenderFill(currPPC, currFB);
				PrintTime(string("Mesh ") + to_string(r->id) + string(" spends: "));

				if (isRenderAABB)
					r->RenderAABB(currPPC, currFB);
			}
		}
		else
		{
			BeginCountingTime();
			RaytracingScene(currPPC, currFB);
			PrintTime("Ray tracing per frame used: ");
		}
		currFB->redraw();
	}
}

void Scene::RenderWireFrame()
{
	fb->ClearBGRZ(0xFFFFFFFF, 0.0f);

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

		if (GlobalVariables::Instance()->isDebugZbuffer)
			currFB->ClearBGRZ(0xFF000000, 0.0f);

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

void Scene::RenderHW()
{
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ppc->SetIntrinsicsHW();
	ppc->SetExtrinsicsHW();

	for(auto t:meshes)
	{
		BeginCountingTime();
		t->RenderHW();
		PrintTime("HW render "+ to_string(t->id) + " spends: ");
	}
}

void Scene::RaytracingScene(PPC* currPPC, FrameBuffer* currFB)
{
	for (int v = 0; v < currFB->h; ++v)
	{
		if (GlobalVariables::Instance()->isDBGRaytracing)
		{
			currFB->DrawRectangle(0, v, currFB->w - 1, v + 1, 0xFFFF0000);
			currFB->redraw();
			Fl::check();

			currFB->DrawRectangle(0, v, currFB->w - 1, v + 1, 0xFFFFFFFF);
		}

		for (int u = 0; u < currFB->w; ++u)
		{
			V3 ray = currPPC->GetRay(u, v);

			// Draw cube map
			currFB->DrawPoint(u, v, cubemap->LookupColor(ray).GetColor());

			// use SBB to prune branches
			// But find it is not efficient
			if (GlobalVariables::Instance()->isUseSBB)
			{
				bool isMissing = true;
				for (auto sbb : raytracingSBB)
				{
					auto [isIntersect, t] = sbb->RaySBB(currPPC->C, ray);
					if (isIntersect)
					{
						isMissing = false;
						break;
					}
				}

				if (isMissing)
				{
					continue;
				}
			}

			// For each meshes, find the closest intersection
			float closestZ = 0.0f;
			TM* shadingMesh = meshes[0];
			PointProperty closestPP(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
			for (auto m : meshes)
			{
				auto [pp, w] = m->RayMeshIntersect(currPPC->C, ray);

				if (w > closestZ)
				{
					// Update point property
					closestZ = w;
					shadingMesh = m;
					closestPP = pp;
				}
			}

			for (auto r : refletors)
			{
				auto [pp, w] = r->RayMeshIntersect(currPPC->C, currPPC->GetRay(u, v));

				if (w > closestZ)
				{
					// Update point property
					closestZ = w;
					shadingMesh = r.get();
					closestPP = pp;
				}
			}

			// Background
			if (FloatEqual(closestZ, 0.0f))
				continue;

			if (!currFB->DepthTest(u, v, closestZ))
				continue;

			auto [color, alpha] = shadingMesh->Shading(currPPC, currFB, u, v, closestZ, closestPP);

			// alpha blending 
			if (!FloatEqual(alpha, 1.0f))
			{
				V3 bgC(0.0f);
				bgC.SetColor(currFB->Get(u, v));
				color = color * alpha + bgC * (1.0f - alpha);
			}

			currFB->DrawPoint(u, v, color.GetColor());
		}
	}
}

void Scene::UpdateBBs()
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
void Scene::RenderBB(PPC* currPPC, FrameBuffer* currFB, TM* reflector)
{
	if (currPPC && currFB && reflector)
	{
		currFB->ClearBGRZ(0x00999999, 0.0f);
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

V3 Scene::RayTracingColor(ray r, hitable_list& obj_list, int depth)
{
	// ray intersect
	hit_record rec;
	if (obj_list.hit(r, 0.001f, FLT_MAX, rec))
	{
		ray scattered;
		V3 attenuation;
		if(depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered))
		{
			V3 rayColor = RayTracingColor(scattered, obj_list, depth + 1);
			V3 ret;
			ret[0] = attenuation[0] * rayColor[0];
			ret[1] = attenuation[1] * rayColor[1];
			ret[2] = attenuation[2] * rayColor[2];
			return ret;
		}
		else
		{
			return V3(0.0f);
		}
	}
	else
	{
		// back ground
		V3 unit_direction = r.direction().UnitVector();
		float t = std::clamp(0.5f * (unit_direction.y() + 1.0f), 0.0f,1.0f);
		return  V3(1.0f) * (1.0f - t) + V3(0.5f, 0.7f, 1.0f) * t;
	}
}

void Scene::RenderRaytracing()
{
	fb->ClearBGRZ(0xFFFFFFFF, 0.0f);

	BeginCountingTime();
	// numble of samples
	int ns = 32;

	for (int v = 0; v < fb->h; ++v)
	{
		if (GlobalVariables::Instance()->isDBGRaytracing)
		{
			fb->DrawRectangle(0, v, fb->w, v + 1, 0xFF0000FF);
			Fl::check();
			fb->redraw();
			fb->DrawRectangle(0, v, fb->w - 1, v, 0xFFFFFFFF);
		}

		for (int u = 0; u < fb->w; ++u)
		{
			V3 col(0.0f);
			int si;
			if (GlobalVariables::Instance()->isOpenMP)
			{
#pragma omp parallel private(si)
				{
#pragma omp for schedule(dynamic) nowait
					for (si = 0; si < ns; ++si)
					{
						float su = float(u) + Random(0.0f,1.0f), sv = float(v) + Random(0.0f, 1.0f);
						ray r = ppc->GetRayWithAperture(su, sv);
						V3 traceColor = RayTracingColor(r, obj_list,0);
#pragma omp critical
						col = col + traceColor;
					}
				}
			}
			else
			{
				for (si = 0; si < ns; ++si)
				{
					float su = float(u) + Random(0.0f, 1.0f), sv = float(v) + Random(0.0f, 1.0f);
					ray r = ppc->GetRayWithAperture(su, sv);
					V3 traceColor = RayTracingColor(r, obj_list,0);
					col = col + traceColor;
				}
			}

			col = col / float(ns);
			col = V3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
			fb->SetGuarded(u, v, col.GetColor());
		}
	}
	PrintTime("Raytracing Time: ");

	fb->redraw();
	fb->SaveAsTiff("images/multisampling.tiff");
}

void Scene::RandomScene()
{
	int n = 500;
	auto &scene_list = obj_list.list;
	scene_list.reserve(n + 1);
	scene_list.push_back(make_shared<sphere>(V3(0.0f, -1000.0f, 0.0f), 1000.0f, make_shared<lambertian>(V3(0.5f))));
	int i = 1;
	for(int a = -11; a < 11; ++a)
	{
		for(int b = -11; b < 11; ++b)
		{
			float choose_mat = Random(0.0f, 1.0f);
			V3 center(float(a) + 0.9f * Random(0.0f, 1.0f), 0.2f,float(b) + 0.9f * Random(0.0f, 1.0f));
			if((center-V3(4.0f,0.2f,0.0f)).Length() > 0.9f)
			{
				if(choose_mat < 0.8f)
				{	// diffuse
					scene_list.push_back(make_shared<sphere>(center, 0.2f, make_shared<lambertian>(V3(Random(0.0f, 1.0f) * Random(0.0f, 1.0f), Random(0.0f, 1.0f)*Random(0.0f, 1.0f), Random(0.0f, 1.0f) * Random(0.0f, 1.0f)))));
				}
				else if(choose_mat < 0.95f)
				{	// metal
					scene_list.push_back(make_shared<sphere>(center, 0.2f, make_shared<metal>(V3(0.5f * (1.0f + Random(0.0f, 1.0f)), 0.5f *(1.0f + Random(0.0f, 1.0f)), Random(0.0f, 0.5f)))));
				}
				else
				{
					scene_list.push_back(make_shared<sphere>(center, 0.2f, make_shared<dielectric>(1.5f)));
				}
			}
		}
	}

	scene_list.push_back(make_shared<sphere>(V3(0.0f, 1.0f, 0.0f), 1.0f, make_shared<dielectric>(1.5f)));
	scene_list.push_back(make_shared<sphere>(V3(-4.0f, 1.0f, 0.0f), 1.0f, make_shared<lambertian>(V3(0.4f,0.2f,0.1f))));
	scene_list.push_back(make_shared<sphere>(V3(4.0f, 1.0f, 0.0f), 1.0f, make_shared<metal>(V3(0.7f,0.6f,0.5f),0.0f)));
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

	l0SM->ClearBGRZ(0xFFFFFFFF, 0.0f);
	l0PPC->PositionAndOrient(L0, meshes[0]->GetCenter(), V3(0.0f, 1.0f, 0.0f));

	// commit to framebuffer
	shadowMaps.push_back(l0SM);
	lightPPCs.push_back(l0PPC);
	UpdateSM();
}

void Scene::PrintTime(const string dbgInfo)
{
	endTime = Clock::now();
	cerr << dbgInfo << std::chrono::duration_cast<chrono::nanoseconds>(endTime - beginTime).count() * 1e-9 << "s" <<
		endl;
}

void Scene::InitDemo()
{
	TM* tm = new TM();
	tm->LoadModelBin("geometry/teapot1K.bin");
	V3 tmC = ppc->C + ppc->GetVD() * 100.0f;
	float tmSize = 100.0f;
	tm->PositionAndSize(tmC, tmSize);

	meshes.push_back(tm);
}

void Scene::Demonstration()
{
	for (int i = 0; i < 100; ++i)
	{
		ppc->C = ppc->C + ppc->a.UnitVector() * 0.1f;
		Render(ppc, fb);
		hwfb->redraw();
		Fl::check();
	}
}
