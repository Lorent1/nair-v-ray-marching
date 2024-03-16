
#include <vector>
#include <chrono>
#include <string>
#include <iostream>
#include <omp.h>

#include "RayMarcher.h"
#include "include/sdf_functions.h"

float4 map(float3 p) {
    float4 scene = float4(1.0f);

    float4 sphere = sdf_sphere(p, float3(0.0f, 2.0f, -3.0f), float3(0.0f, 0.0f, 1.0f), 2.0f);
    float4 sphere2 = sdf_sphere(p, float3(0.0f, 0.0f, 0.0f), float3(0.67f, 0.87f, 0.87f), 5.7f);
    float4 sphere3 = sdf_sphere(p, float3(0.0f, -1.5f, -5.0f), float3(0.67f, 0.87f, 0.87f), 2.0f);

    float4 rbox = sdf_roundBox(p, float3(0.0f, -1.0f, 0.0f), float3(0.25f, 0.79f, 0.87f), float3(5.0f), 0.5f);

    float4 plain = sdf_plane(p, float3(1.0f, 1.0f, 1.0f), float3(0.0f, 1.0f, 0.0f), 1.5f, true);

    float4 sponge = sdf_mengerSponge(p, float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.55f, 0.85f), float3(1.0f), 2.0f);

    float4 mandelbulb = sdf_mandelbulb(p, float3(0.0f, 7.0f, 0.0f), float3(0.25f, 0.79f, 0.87f), 7, 3.0f);

    scene = smoothSubtraction(sphere, sponge, 0.8f);

    float4 tmp = smoothSubtraction(sphere2, rbox, 0.95f);
    scene = unionSDF(tmp, scene);

    tmp = smoothUnion(plain, sphere3, 0.9f);
    scene = unionSDF(scene, tmp);

    scene = unionSDF(mandelbulb, scene);

    return scene;
}

#include "include/light_functions.h"

static inline uint32_t RealColorToUint32(float4 real_color){
    int red = std::max(0, std::min(255, (int)(real_color[0] * 255.0f)));
    int green = std::max(0, std::min(255, (int)(real_color[1] * 255.0f)));
    int blue = std::max(0, std::min(255, (int)(real_color[2] * 255.0f)));
    int alpha = std::max(0, std::min(255, (int)(real_color[3] * 255.0f)));

    return red | (green << 8) | (blue << 16) | (alpha << 24);
}

float3x3 getCam(float3 ro, float3 lookAt) {
	float3 camF = normalize(lookAt - ro);
	float3 camR = normalize(cross(float3(0.0f, 1.0f, 0.0f), camF));
	float3 camU = cross(camF, camR);

	return make_float3x3_by_columns(camR, camU, camF);
}

float2 getUV(float2 offset, int x, int y, int width, int heigth) {
    float ratio = (float)width / heigth;
    return ((float2(x, y) - offset) * 2.0f / float2(width, heigth) - 1.0f) * float2(ratio, 1.0f);
}

float3 RayMarcher::render(float2 uv, int x, int y){
    float3 ro = float3(_ro.x, _ro.y, _ro.z);
    float3 rd = getCam(ro, _lookAt) * normalize(float3(uv.x, uv.y, FOV));

    float3 background = float3(0.4f, 0.7f, 0.9f);
    float3 pixel = float3(0.0f);
    float4 pixelInfo; // color(r, g, b), distance;

    float3 p;
    float t = 0.0f;

    for (int i = 0; i < 500; i++) {
        p = ro + rd * t;
        pixelInfo = map(p);

        t += pixelInfo.w; // distance
        if (t > 50 || pixelInfo.w < 1e-3) break;
    }

    if (t < 50){
        pixel = getLight(p, rd, float3(pixelInfo.x, pixelInfo.y, pixelInfo.z));
        pixel = mix(pixel, background, 1.0f - exp(-1e-4 * t * t * t));
    }else {
        pixel = background/pixelInfo.w - max(0.9f * rd.y, 0.0f);
    }

    return pixel;
}


void RayMarcher::kernel2D_RayMarchAAX1(uint32_t* out_color, uint32_t width, uint32_t height){
    #pragma omp for collapse(2)       
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            float2 uv = getUV(float2(0.0f), x, y, width, height);
            float3 pixel = render(uv, x, y);

            out_color[y * width + x] = RealColorToUint32(float4(pixel.x, pixel.y, pixel.z, 1.0f));
        }
    }
}

void RayMarcher::kernel2D_RayMarchAAX4(uint32_t* out_color, uint32_t width, uint32_t height){
    #pragma omp for collapse(2)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float4 e = float4(0.125f, -0.125f, 0.375f, -0.375f);
            float3 pixel = render(getUV(float2(e.x, e.z), x, y, width, height), x, y)
                + render(getUV(float2(e.y, e.w), x, y, width, height), x, y)
                + render(getUV(float2(e.w, e.x), x, y, width, height), x, y)
                + render(getUV(float2(e.z, e.y), x, y, width, height), x, y);
            pixel /= 4.0f;

            out_color[y * width + x] = RealColorToUint32(float4(pixel.x, pixel.y, pixel.z, 1.0f));
        }
    }
}


void RayMarcher::RayMarch(uint32_t* out_color, uint32_t width, uint32_t height, int aliasingType){
    auto start = std::chrono::high_resolution_clock::now();
    #pragma omp parallel
    {
        switch(aliasingType){
        case 4:
            kernel2D_RayMarchAAX4(out_color, width, height);
            break;
        default:
            kernel2D_RayMarchAAX1(out_color, width, height);
        }
    }
    rayMarchTime = float(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000.f;
}

void RayMarcher::GetExecutionTime(const char* a_funcName, float a_out[4]){
    if (std::string(a_funcName) == "RayMarch")
        a_out[0] = rayMarchTime;
}
