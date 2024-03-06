
#include <vector>
#include <chrono>
#include <string>

#include "RayMarcher.h"

float4 unionSDF(float4 a, float4 b) {
    return a.w < b.w ? a : b;
}

float4 map(float3 p) {

    float sp = length(p) - 0.5f;
    float pl = dot(p, normalize(float3(0.0f,1.0f,0.0f))) + 1.5f;
    float4 sphere = float4(1.0f, 1.0f, 1.0f, sp);
    float3 color = float3(0.2f + 0.4f * mod(floor(p.x) + floor(p.z), 2.0f));
    float4 plain = float4(color.x, color.y, color.z, pl);

    return unionSDF(plain, sphere);
}

float3 EstimateNormal(float3 z, float eps) {
    float3 z1 = z + float3(eps, 0, 0);
    float3 z2 = z - float3(eps, 0, 0);
    float3 z3 = z + float3(0, eps, 0);
    float3 z4 = z - float3(0, eps, 0);
    float3 z5 = z + float3(0, 0, eps);
    float3 z6 = z - float3(0, 0, eps);
    float dx = map(z1).w - map(z2).w;
    float dy = map(z3).w - map(z4).w;
    float dz = map(z5).w - map(z6).w;
    return normalize(float3(dx, dy, dz) / (2.0f * eps));
}

float getSoftShadow(float3 p, float3 lightPos) {
    float res = 1.0f;
    float dist = 0.01f;
    float lightSize = 0.03f;

    for (int i = 0; i < 100 / 5; i++) {
        float hit = map(p + lightPos * dist).w;

        res = min(res, hit / (dist * lightSize));
        dist += hit;
        if (hit < 1e-3 * 1e-3 || dist > 50) break;
    }

    return clamp(res, 0.0f, 1.0f);
}

float getAmbientOcclusion(float3 p, float3 normal) {
    float occ = 0.0f;
    float weight = 1.0f;

    for (int i = 0; i < 8; i++) {
        float len = 0.01f + 0.02f * float(i * i);
        float dist = map(p + normal * len).x;
        occ += (len - dist) * weight;
        weight *= 0.85f;
    }
    return 1.0f - clamp(0.6f * occ, 0.0f, 1.0f);
}

float3 getLight(float3 pos, float3 rd, float3 color) {
    float3 lightPos = float3(2.0f, 20.0f, -5.0f);
    float3 L = normalize(lightPos - pos);
    float3 N = EstimateNormal(pos, 1e-3f);
    float3 V = -1 * rd;
    float3 R = reflect(-1 * L, N);

    float3 specColor = float3(0.5f);
    float3 specular = 1.3f * specColor * pow(clamp(dot(R, V), 0.0f, 1.0f), 10.0f);
    float3 ambient = 0.05f * color;
    float3 fresnel = 0.15f * color * pow(1.0f + dot(rd, N), 3.0f);

    float shadow = getSoftShadow(pos + N * 0.02f, normalize(lightPos));
    float occ = getAmbientOcclusion(pos, N);

    float3 dif = color * clamp(dot(N, L), 0.1f, 1.0f);

    return (ambient + fresnel) * occ + (dif + specular * occ) * shadow;
}


static inline uint32_t RealColorToUint32(float4 real_color){
    int red = std::max(0, std::min(255, (int)(real_color[0] * 255.0f)));
    int green = std::max(0, std::min(255, (int)(real_color[1] * 255.0f)));
    int blue = std::max(0, std::min(255, (int)(real_color[2] * 255.0f)));
    int alpha = std::max(0, std::min(255, (int)(real_color[3] * 255.0f)));

    return red | (green << 8) | (blue << 16) | (alpha << 24);
}


void RayMarcher::kernel2D_RayMarch(uint32_t* out_color, uint32_t width, uint32_t height){        
    for (uint32_t y = 0; y < height; y++){
        for (uint32_t x = 0; x < width; x++){
            float ratio = (float)width / height;
            float3 background = float3(0.5f, 0.8f, 0.9f);
            float4 pixelInfo; // color(r, g, b), distance;
            float2 uv = (float2((float)x, (float)y) * 2.0f / float2((float)width, (float)height) - 1.0f) * float2(ratio, 1.0f);
            float3 rd = camera * normalize(float3(uv.x, uv.y, 1.0f));
            float3 p;
            float t = 0.0f;
            float3 pixel = float3(0.0f);

            for (int i = 0; i < 500; i++) {
                p = ro + rd * t;
                pixelInfo = map(p);

                t += pixelInfo.w; // distance
                if (t > 50 || pixelInfo.w < 1e-3) break;
            }

            if (t < 50) {
                pixel = getLight(p, rd, float3(pixelInfo.x, pixelInfo.y, pixelInfo.z));
                pixel = mix(pixel, background, 1.0f - exp(-1e-4 * t * t * t));
            }
            else {
                pixel = background/pixelInfo.w - max(0.9f * rd.y, 0.0f);
            }

            out_color[y * width + x] = RealColorToUint32(float4(pixel.x, pixel.y, pixel.z, 1.0f));
        }
    }
}


void RayMarcher::RayMarch(uint32_t* out_color, uint32_t width, uint32_t height){
    auto start = std::chrono::high_resolution_clock::now();
    kernel2D_RayMarch(out_color, width, height);
    rayMarchTime = float(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000.f;
}

void RayMarcher::GetExecutionTime(const char* a_funcName, float a_out[4]){
    if (std::string(a_funcName) == "RayMarch")
        a_out[0] = rayMarchTime;
}
