#include "GlobalVariables.h"

GlobalVariables* GlobalVariables::_instance = nullptr;

GlobalVariables* GlobalVariables::Instance()
{
	if (_instance == nullptr)
	{
		_instance = new GlobalVariables();
	}

	return _instance;
}

GlobalVariables::GlobalVariables()
{
	isHighResolution = true;
	highResoW = 1020;
	highResoH = 720;
	resoW = 640;
	resoH = 480;

	tmAnimationID = 0;
	dbgParameter = 10.0f;
	curScene = nullptr;
	isRenderProjectedTexture = true;
	depthTest = true;
	lodTexture = false;
	projectedTextureName = "images/jojo.tiff";
	checkerBoxTexName = "images/Checkerboard_pattern.tiff";

	debugZbuffer = false;
	isSaveLodTextures = false;
	isLight = true;
	isShadow = false;
	isRefraction = false;
	refractRatio = 0.8f;

	isRecording = false;
}
