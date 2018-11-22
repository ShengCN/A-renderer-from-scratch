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
	isHighResolution = false;
	highResoW = 1020;
	highResoH = 720;
	resoW = 640;
	resoH = 480;
	isRayTracing = false;
	isDBGRaytracing = true;
	isUseSBB = true;
	isPrintFPS = false;

	tmAnimationID = 0;
	dbgParameter = 10.0f;
	curScene = nullptr;
	isRenderProjectedTexture = true;
	depthTest = true;
	lodTexture = true;
	projectedTextureName = "images/jojo.tiff";
	checkerBoxTexName = "images/Checkerboard_pattern.tiff";

	isDebugZbuffer = false;
	isSaveLodTextures = false;
	isLight = true;
	isShadow = false;
	isRefraction = false;
	isWireFrame = false;
	refractRatio = 1.15f;
	
	string cubemapFolder = "images/cubemaps/";

	cubemapFiles = {
		cubemapFolder + "right.tiff",
		cubemapFolder + "left.tiff",
		cubemapFolder + "ground.tiff",
		cubemapFolder + "top.tiff",
		cubemapFolder + "front.tiff",
		cubemapFolder + "back.tiff"
	};

	isCubemapMipmap = false;

	isRecording = false;
	isOpenMP = true;
}
