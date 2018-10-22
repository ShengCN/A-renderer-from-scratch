#pragma once
#include "ppc.h"
#include "TM.h"
#include "scene.h"

class GlobalVariables
{
public:
	static GlobalVariables* Instance();

	// Global variables
	Scene *curScene;
	bool isRenderProjectedTexture;
	string projectedTextureName;
	bool depthTest, lodTexture;
	bool isRecording;
	bool debugZbuffer;

	vector<const string> cubeMapFiles;
private:
	GlobalVariables();
	static GlobalVariables* _instance;
};

