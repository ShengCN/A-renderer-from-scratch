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
using std::vector;
using std::unique_ptr;
using Clock = std::chrono::high_resolution_clock;

class Scene
{
public:
	GUI* gui;
	FrameBuffer* fb, *fb3, *fbp;
	std::vector<shared_ptr<FrameBuffer>> textures;
	std::vector<shared_ptr<PPC>> lightPPCs;
	std::vector<shared_ptr<FrameBuffer>> shadowMaps;
	PPC* ppc, *ppc3, *projectPPC;
	vector<TM*> meshes;
	vector<shared_ptr<TM>> refletors;
	vector<shared_ptr<BillBoard>> sceneBillboard;
	shared_ptr<CubeMap> cubemap;
	vector<shared_ptr<SBB>> raytracingSBB;	// Sphere BB
	hitable_list obj_list;

	Scene();
	void DBG();
	void DBGZBuffer(string outFile,FrameBuffer *curfb);
	void Render();				// render all triangles in the scene
	void Render(PPC *currPPC, FrameBuffer *currFB);
	void RenderWireFrame();
	void RenderZbuffer(PPC *currPPC, FrameBuffer *currFB);
	void UpdateSM();

	void RaytracingScene(PPC *ppc, FrameBuffer *fb);

	// HW5, Implement part of the paper
	// https://www.cs.purdue.edu/cgvlab/papers/popescu/popescuGemEG06.pdf
	// Render all other tm except id mesh to id's billboards
	void UpdateBBs();
	void RenderBB(PPC *ppc, FrameBuffer *fb, TM *reflectors);

	V3 GetSceneCenter();
	~Scene();

	// Ray tracing
	V3 RayTracingColor(ray r, hitable_list &obj_list, int depth);
	void RenderRaytracing();

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
	chrono::time_point<chrono::steady_clock> beginTime;
	chrono::time_point<chrono::steady_clock> endTime;
};

extern Scene* scene;
