#pragma once
#include <vector>
#include <memory>
#include "gui.h"
#include "framebuffer.h"
#include "ppc.h"
#include "TM.h"
using std::vector;
using std::unique_ptr;

class Scene
{
public:
	GUI* gui;
	FrameBuffer* fb, *fb3;
	std::vector<shared_ptr<FrameBuffer>> textures;
	PPC* ppc, *ppc3;
	vector<TM*> meshes;

	Scene();
	void DBG();
	void Render();				// render all triangles in the scene
	void Render(PPC *currPPC, FrameBuffer *currFB);
	void RenderTexture(PPC *currPPC, FrameBuffer *currFB);
	void RenderWireFrame();

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
	int LookupLodLevel();
};

extern Scene* scene;
