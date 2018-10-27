#pragma once
#include "ppc.h"
#include "TM.h"
#include "scene.h"

class GlobalVariables
{
public:
	static GlobalVariables* Instance();

	// Global variables
	bool isHighResolution;
	int highResoW, highResoH;
	int resoW, resoH;
	Scene *curScene;
	bool isRenderProjectedTexture;
	string projectedTextureName;
	bool depthTest, lodTexture;
	bool isRecording;
	bool debugZbuffer;
	bool isSaveLodTextures;
	bool isLight;
	float dbgParameter;

	// For HW5
	bool isRefraction;
	float refractRatio;
	string checkerBoxTexName;
	bool isShadow;
	int tmAnimationID;

private:
	GlobalVariables();
	static GlobalVariables* _instance;
};

