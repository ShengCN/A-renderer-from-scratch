#pragma once

#include <Cg/cgGL.h>
#include <Cg/cg.h>
#include <string>

// two classes defining the interface between the CPU and GPU

// models part of the CPU-GPU interface that is independent of specific shaders
class CGInterface
{
public:
	CGprofile vertexCGprofile; // vertex shader profile of GPU
	CGprofile geometryCGprofile; // geometry shader profile of GPU
	CGprofile pixelCGprofile; // pixel shader profile of GPU
	CGcontext cgContext;
	void PerSessionInit(); // per session initialization
	CGInterface(); // constructor
	void EnableProfiles(); // enable GPU rendering
	void DisableProfiles(); // disable GPU rendering
};


// models the part of the CPU-GPU interface for a specific shader
//        here there is a single shader "ShaderOne"
//  a shader consists of a vertex, a geometry, and a pixel (fragment) shader
// fragment == pixel; shader == program; e.g. pixel shader, pixel program, fragment shader, fragment program, vertex shader, etc.
class ShaderOneInterface
{
	CGprogram geometryProgram; // the geometry shader to be used for the "ShaderOne"
	CGprogram vertexProgram;
	CGprogram fragmentProgram;
	// uniform parameters, i.e parameters that have the same value for all geometry rendered
	CGparameter vertexModelViewProj; // a matrix combining projection and modelview matrices
	CGparameter vertexMorphFraction;

	CGparameter geometryModelViewProj; // geometry shader
	CGparameter fragmentKa; // ambient coefficient for fragment shader
	CGparameter fragmentLightPos, fragmentPPCC;
	CGparameter fragmentIsST;
	CGparameter fragmentTex0;
	CGparameter fragmentCubemapTex;
	CGparameter fragmentIsCubemap;

	// Render reflection using bb
	CGparameter fragmentBB;
public:
	ShaderOneInterface()
	{
	};
	bool PerSessionInit(CGInterface* cgi, const std::string shaderOneFile); // per session initialization
	void BindPrograms(); // enable geometryProgram, vertexProgram, fragmentProgram
	void PerFrameInit(int hasST, int isCubemap, const std::string tex0File); // set uniform parameter values, etc.

	void PerFrameDisable(); // disable programs
};
