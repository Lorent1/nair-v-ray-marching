#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdint>

#include "LiteMath.h"

using namespace LiteMath;

class RayMarcher {
public:

	RayMarcher(){
		_lookAt = float3(0.0f, 0.0f, 0.0f);
		_ro = float3(0.0f, 0.0f, 0.0f);
	}

	void setCamera(float3 ro, float3 lookAt, float fov) { 
		_lookAt = lookAt;
		FOV = fov;
		_ro = ro; 
	};

	virtual void kernel2D_RayMarchAAX1(uint32_t* out_color, uint32_t width, uint32_t height);
	virtual void kernel2D_RayMarchAAX4(uint32_t* out_color, uint32_t width, uint32_t height);
	virtual void RayMarch(uint32_t* out_color [[size("width*height")]], uint32_t width, uint32_t height, int aliasingType);
	virtual float3 render(float2 uv, int x, int y);
	virtual void CommitDeviceData() {}                                       // will be overriden in generated class
	virtual void UpdateMembersPlainData() {}                                 // will be overriden in generated class (optional function)
	//virtual void UpdateMembersVectorData() {}                              // will be overriden in generated class (optional function)
	//virtual void UpdateMembersTexureData() {}                              // will be overriden in generated class (optional function)
	virtual void GetExecutionTime(const char* a_funcName, float a_out[4]);   // will be overriden in generated class
protected:
	float3 _lookAt;
	float3 _ro;
	float rayMarchTime;
	float FOV;
};