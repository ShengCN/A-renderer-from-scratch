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
	curScene = nullptr;
	isRenderProjectedTexture = true;
	depthTest = true;
	lodTexture = true;
	projectedTextureName = "images/jojo.tiff";
	debugZbuffer = false;
	isSaveLodTextures = false;
	isLight = false;
	isRefraction = true;
	refractRatio = 0.7f;

	isRecording = false;
}
