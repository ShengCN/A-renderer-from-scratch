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
	debugZbuffer = true;

	cubeMapFiles.push_back("images/front.tiff");
	cubeMapFiles.push_back("images/top.tiff");
	cubeMapFiles.push_back("images/back.tiff");
	cubeMapFiles.push_back("images/ground.tiff");
	cubeMapFiles.push_back("images/left.tiff");
	cubeMapFiles.push_back("images/right.tiff");

	isRecording = true;
}
