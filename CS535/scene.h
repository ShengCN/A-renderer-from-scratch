#pragma once
#include <vector>
#include <memory>
#include "gui.h"
#include "framebuffer.h"
#include "ppc.h"
#include "TM.h"
#include "cubemap.h"
using std::vector;
using std::unique_ptr;

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
	shared_ptr<CubeMap> cubemap;

	Scene();
	void DBG();
	void DBGZBuffer(string outFile,FrameBuffer *curfb);
	void Render();				// render all triangles in the scene
	void Render(PPC *currPPC, FrameBuffer *currFB);
	void RenderWireFrame();
	void RenderZbuffer(PPC *currPPC, FrameBuffer *currFB);
	void UpdateSM();

	V3 GetSceneCenter();
	~Scene();

private:
	bool DBGFramebuffer();
	bool DBGV3();
	bool DBGM3();
	bool DBGAABB();
	bool DBGPPC();
	void Demonstration();
	void InitDemo();
	bool isRenderAABB;
	void InitializeLights();
};

extern Scene* scene;
