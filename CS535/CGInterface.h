#pragma once

#include <Cg/cgGL.h>
#include <Cg/cg.h>
#include <string>
#include <vector>

class TM;

struct uniformVariables
{
	int hasST;
	int isCubemap;
	int isBox;
	int isGround;
	std::string tex0File;
	std::string ltc_1;
	std::string ltc_2;
};

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
	CGparameter fragmentMF;

	CGparameter geometryModelViewProj; // geometry shader
	CGparameter fragmentKa; // ambient coefficient for fragment shader
	CGparameter fragmentPPCC;
	CGparameter fragmentIsST;
	CGparameter fragmentTex0;
	CGparameter fragmentCubemapTex;
	CGparameter fragmentIsCubemap;

	// Hard coded
	// The other two boxes
	CGparameter fragmentIsGround;
	CGparameter fragmetGroundHeight;
	CGparameter fragmentBRDF_ltc1;	// ltc1 texture
	CGparameter fragmentBRDF_ltc2;	// ltc2 texture
	CGparameter fragmentPoints;		// rectangular points

public:
	ShaderOneInterface()
	{
	};
	bool PerSessionInit(CGInterface* cgi, const std::string shaderOneFile); // per session initialization
	void BindPrograms(); // enable geometryProgram, vertexProgram, fragmentProgram
	void PerFrameInit(uniformVariables &uniforms); // set uniform parameter values, etc.

	void PerFrameDisable(); // disable programs
};
