#pragma once
#include <vector>
#include <memory>

#include "gui.h"
#include "framebuffer.h"
#include "ppc.h"
#include "TM.h"
#include "cubemap.h"
#include "BillBoard.h"
#include "SBB.h"
#include <chrono>
#include "hitable_list.h"
#include "CGInterface.h"
using std::vector;
using std::unique_ptr;
using Clock = std::chrono::high_resolution_clock;

class Scene
{
public:
	GUI* gui;
	shared_ptr<FrameBuffer> fb, fb3, fbp, gpufb, leftfb, rightfb;
	shared_ptr<PPC> ppc, ppc3, projectPPC, leftppc, rightppc;

	std::vector<shared_ptr<FrameBuffer>> textures;
	std::vector<shared_ptr<PPC>> lightPPCs;
	std::vector<shared_ptr<FrameBuffer>> shadowMaps;
	std::vector<shared_ptr<TM>> meshes;
	std::vector<shared_ptr<TM>> reflectors;
	std::vector<shared_ptr<BillBoard>> sceneBillboard;
	std::shared_ptr<CubeMap> cubemap;
	std::vector<shared_ptr<SBB>> raytracingSBB;	// Sphere BB
	hitable_list obj_list;
	float ka, mf;

	Scene();
	void DBG();
	void DBGZBuffer(string outFile,FrameBuffer *curfb);
	void Render();				// render all triangles in the scene
	void Render(shared_ptr<PPC> currPPC, shared_ptr<FrameBuffer> currFB);
	void RenderWireFrame();
	void RenderZbuffer(shared_ptr<PPC> currPPC, shared_ptr<FrameBuffer> currFB);
	void UpdateSM();
	void RenderGPU();
	void RenderGPUWireframe();
	void ReloadCG();

	// HW5, Implement part of the paper
	// https://www.cs.purdue.edu/cgvlab/papers/popescu/popescuGemEG06.pdf
	// Render all other tm except id mesh to id's billboards
	void UpdateBBs();
	void RenderBB(shared_ptr<PPC> curPPC, shared_ptr<FrameBuffer> curFB, shared_ptr<TM> reflectors);
	void UpdateBBsGPU();
	void RenderBBGPU(shared_ptr<PPC> curPPC, shared_ptr<FrameBuffer> curFB, shared_ptr<TM> reflector);

	V3 GetSceneCenter();
	~Scene();

	// Ray tracing
	V3 RayTracingColor(ray r, hitable_list &obj_list, int depth);
	void RenderRaytracing();
	void RandomScene();
	void RaytracingScene(shared_ptr<PPC> ppc, shared_ptr<FrameBuffer> fb);

	// panaroma
	void ShowPano();

	// Cubemaps
	void InitializeCubeMap();

private:
	bool isRenderAABB;

	bool DBGFramebuffer();
	bool DBGV3();
	bool DBGM3();
	bool DBGAABB();
	bool DBGPPC();
	void Demonstration();
	void InitDemo();
	void InitializeLights();

	void BeginCountingTime() { beginTime = Clock::now(); };
	void PrintTime(const string dbgInfo);
	void PrintTime(const string fbname, shared_ptr<FrameBuffer> curfb);
	chrono::time_point<chrono::steady_clock> beginTime;
	chrono::time_point<chrono::steady_clock> endTime;
};

extern Scene* scene;
