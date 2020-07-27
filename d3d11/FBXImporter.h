#pragma once

//This was causing certain temlplate functions to throw linker errors without the define
//NOTE: This is also causing some trouble with the linker. Maybe get rid of the below #pragma 
#define FBXSDK_SHARED 

#pragma comment(lib, "libfbxsdk.lib")

#include <fbxsdk.h>
#include "RenderSystem.h"
#include "Actor.h"

using namespace fbxsdk;

class FBXImporter
{
public:
	static void Init();
	static bool Import(const char* filename, ModelData& data);
};

