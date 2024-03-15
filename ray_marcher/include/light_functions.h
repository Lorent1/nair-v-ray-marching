#pragma once

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
    return normalize(float3(dx, dy, dz));
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
    float3 N = EstimateNormal(pos, 1e-3);
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
