#pragma once
#include "ppc.h"
#include "TM.h"
#include "scene.h"

class GlobalVariables
{
public:
	Scene *curScene;
	bool isRenderProjectedTexture;
	static GlobalVariables* Instance();
private:
	GlobalVariables();
	static GlobalVariables* _instance;
};

