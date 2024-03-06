#pragma once


#include <vector>
#include <iostream>
#include <fstream>
#include <cstdint>

#include <LiteMath.h>

using namespace LiteMath;

class RayMarcher {
public:

	RayMarcher(){
		const float4x4 view = lookAt(float3(0, 1.5, -3), float3(0, 0, 0), float3(0, 1, 0)); // pos, look_at, up
		const float4x4 proj = perspectiveMatrix(90.0f, 1.0f, 0.1f, 100.0f);
		m_worldViewInv = inverse4x4(view);
		m_worldViewProjInv = inverse4x4(proj);
	}

	void SetWorldViewMatrix(const float4x4& a_mat) { m_worldViewInv = inverse4x4(a_mat); }

	virtual void kernel2D_RayMarch(uint32_t* out_color, uint32_t width, uint32_t height);
	virtual void RayMarch(uint32_t* out_color [[size("width*height")]], uint32_t width, uint32_t height);
	virtual void CommitDeviceData() {}                                       // will be overriden in generated class
	virtual void UpdateMembersPlainData() {}                                 // will be overriden in generated class (optional function)
	//virtual void UpdateMembersVectorData() {}                              // will be overriden in generated class (optional function)
	//virtual void UpdateMembersTexureData() {}                              // will be overriden in generated class (optional function)
	virtual void GetExecutionTime(const char* a_funcName, float a_out[4]);   // will be overriden in generated class
protected:
	float4x4 m_worldViewProjInv;
	float4x4 m_worldViewInv;
	float    rayMarchTime;
};