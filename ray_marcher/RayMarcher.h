#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdint>

#include "LiteMath.h"

using namespace LiteMath;

#include "file_input/file_input.h"

class RayMarcher {
public:

	RayMarcher(){
		float3 ro = float3(0.0f, 2.0f, -3.0f);
		float3x3 camera = getCam(ro, float3(0, 0, 0));
	}

	float3x3 getCam(float3 ro, float3 lookAt) {
		float3 camF = normalize(float3(lookAt - ro));
		float3 camR = normalize(cross(float3(0.0f, 1.0f, 0.0f), camF));
		float3 camU = cross(camF, camR);

		return make_float3x3_by_columns(camR, camU, camF);
	}

	void setCamera(float3 r, float3 lookAt, float fov) { ro = r; camera = getCam(r, lookAt); FOV = fov; };
	void setCamera(std::string path) { float3 lookAt; InputJson::read_camera(path, &ro, &lookAt, &FOV); camera = getCam(ro, lookAt); };


	virtual void kernel2D_RayMarch(uint32_t* out_color, uint32_t width, uint32_t height);
	virtual void RayMarch(uint32_t* out_color [[size("width*height")]], uint32_t width, uint32_t height);
	virtual void CommitDeviceData() {}                                       // will be overriden in generated class
	virtual void UpdateMembersPlainData() {}                                 // will be overriden in generated class (optional function)
	//virtual void UpdateMembersVectorData() {}                              // will be overriden in generated class (optional function)
	//virtual void UpdateMembersTexureData() {}                              // will be overriden in generated class (optional function)
	virtual void GetExecutionTime(const char* a_funcName, float a_out[4]);   // will be overriden in generated class
protected:
	float    rayMarchTime;
	float FOV;
	float3x3 camera;
	float3 ro;
};