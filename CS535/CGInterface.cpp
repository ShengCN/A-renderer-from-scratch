//#define GEOM_SHADER

#include "CGInterface.h"
#include "v3.h"
#include "scene.h"

#include <iostream>
#include "GlobalVariables.h"
#include <GL/glext.h>
using namespace std;
#define GEOM_SHADER

CGInterface::CGInterface()
{
};

void CGInterface::PerSessionInit()
{
	glEnable(GL_DEPTH_TEST);

	CGprofile latestVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
	CGprofile latestGeometryProfile = cgGLGetLatestProfile(CG_GL_GEOMETRY);
	CGprofile latestPixelProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);

#ifdef GEOM_SHADER
	if (latestGeometryProfile == CG_PROFILE_UNKNOWN)
	{
		cerr << "ERROR: geometry profile is not available" << endl;
		exit(0);
	}

	cgGLSetOptimalOptions(latestGeometryProfile);
	CGerror Error = cgGetError();
	if (Error)
	{
		cerr << "CG ERROR: " << cgGetErrorString(Error) << endl;
	}
#endif

	cout << "Info: Latest GP Profile Supported: " << cgGetProfileString(latestGeometryProfile) << endl;

	geometryCGprofile = latestGeometryProfile;

	cout << "Info: Latest VP Profile Supported: " << cgGetProfileString(latestVertexProfile) << endl;
	cout << "Info: Latest FP Profile Supported: " << cgGetProfileString(latestPixelProfile) << endl;

	vertexCGprofile = latestVertexProfile;
	pixelCGprofile = latestPixelProfile;
	cgContext = cgCreateContext();
}

bool ShaderOneInterface::PerSessionInit(CGInterface* cgi, const std::string shaderOneFile)
{
	if (shaderOneFile.empty())
	{
		cerr << "Shader one file has not set up! \n";
		return false;
	}

#ifdef GEOM_SHADER
	geometryProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE,
	                                          shaderOneFile.c_str(), cgi->geometryCGprofile, "GeometryMain", nullptr);
	if (geometryProgram == nullptr)
	{
		CGerror Error = cgGetError();
		cerr << "Shader One Geometry Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
		cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
		return false;
	}
#endif

	vertexProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE,
	                                        shaderOneFile.c_str(), cgi->vertexCGprofile, "VertexMain", nullptr);
	if (vertexProgram == nullptr)
	{
		CGerror Error = cgGetError();
		cerr << "Shader One Vertex Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
		cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
		return false;
	}

	fragmentProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE,
	                                          shaderOneFile.c_str(), cgi->pixelCGprofile, "FragmentMain", nullptr);
	if (fragmentProgram == nullptr)
	{
		CGerror Error = cgGetError();
		cerr << "Shader One Fragment Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
		cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
		return false;
	}

	// load programs
#ifdef GEOM_SHADER
	cgGLLoadProgram(geometryProgram);
#endif
	cgGLLoadProgram(vertexProgram);
	cgGLLoadProgram(fragmentProgram);

	// Vertex shader
	vertexModelViewProj = cgGetNamedParameter(vertexProgram, "modelViewProj");
	vertexMorphFraction = cgGetNamedParameter(vertexProgram, "Mf");

	// Geometry shader
	geometryModelViewProj = cgGetNamedParameter(geometryProgram, "modelViewProj");

	// Fragment shader
	fragmentPPCC = cgGetNamedParameter(fragmentProgram, "ppc_C");
	fragmentLightPos = cgGetNamedParameter(fragmentProgram, "light_position");
	fragmentIsST = cgGetNamedParameter(fragmentProgram, "hasST");
	fragmentTex0 = cgGetNamedParameter(fragmentProgram, "tex");
	fragmentCubemapTex = cgGetNamedParameter(fragmentProgram, "env");
	fragmentIsCubemap = cgGetNamedParameter(fragmentProgram, "isCubemap");
	fragmentBB = cgGetNamedParameter(fragmentProgram, "bbpos");

	return true;
}

void ShaderOneInterface::PerFrameInit(int hasST, int isCubemap, const std::string tex0File)
{
	//set parameters
	if(isCubemap)
	{
		cgGLSetStateMatrixParameter(vertexModelViewProj,
			CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

		cgGLSetStateMatrixParameter(
			geometryModelViewProj,
			CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
	}
	else
	{
		cgGLSetStateMatrixParameter(vertexModelViewProj,
			CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

		cgGLSetStateMatrixParameter(
			geometryModelViewProj,
			CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
	}

	// Vertex Shader
	cgSetParameter1f(vertexMorphFraction, GlobalVariables::Instance()->curScene->mf);

	// Fragment Shader
	auto curScene = GlobalVariables::Instance()->curScene;
	cgSetParameter3fv(fragmentLightPos, (float*)&(curScene->lightPPCs[0]->C));
	cgSetParameter3fv(fragmentPPCC, (float*)&(curScene->ppc->C));
	cgSetParameter1i(fragmentIsST, hasST);
	cgSetParameter1i(fragmentIsCubemap, isCubemap);

	if(hasST)
	{
		cgGLSetTextureParameter(fragmentTex0, FrameBuffer::gpuTexID.at(tex0File));
		cgGLEnableTextureParameter(fragmentTex0);
	}

	cgGLSetTextureParameter(fragmentCubemapTex, FrameBuffer::gpuTexID.at(GlobalVariables::Instance()->cubemapFiles[0]));
	cgGLEnableTextureParameter(fragmentCubemapTex);
}

void ShaderOneInterface::PerFrameDisable()
{
}


void ShaderOneInterface::BindPrograms()
{
#ifdef GEOM_SHADER
	cgGLBindProgram(geometryProgram);
#endif
	cgGLBindProgram(vertexProgram);
	cgGLBindProgram(fragmentProgram);
}

void CGInterface::DisableProfiles()
{
	cgGLDisableProfile(vertexCGprofile);
#ifdef GEOM_SHADER
	cgGLDisableProfile(geometryCGprofile);
#endif
	cgGLDisableProfile(pixelCGprofile);
}

void CGInterface::EnableProfiles()
{
	cgGLEnableProfile(vertexCGprofile);
#ifdef GEOM_SHADER
	cgGLEnableProfile(geometryCGprofile);
#endif
	cgGLEnableProfile(pixelCGprofile);
}

