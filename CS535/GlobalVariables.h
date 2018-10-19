#pragma once
#include "ppc.h"
#include "TM.h"
#include "scene.h"

class GlobalVariables
{
public:
	Scene *curScene;

	static GlobalVariables* Instance();
private:
	GlobalVariables();
	static GlobalVariables* _instance;
};

