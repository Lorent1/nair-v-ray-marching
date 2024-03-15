#pragma once

float3 getColor(float3 c1, float3 c2, float d1, float d2){
    return (d1 * c2 + d2 * c1) / (d1 + d2);
}

float4 intersectSDF(float4 a, float4 b) {
    return a.w > b.w ? a : b;
}

float4 unionSDF(float4 a, float4 b) {
    return a.w < b.w ? a : b;
}

float4 smoothUnion(float4 d1, float4 d2, float k){
    float h = clamp(0.5f + 0.5f * (d1.w - d2.w) / k, 0.0f, 1.0f);
    float d = mix(d1.w, d2.w, h) - k * h * (1.0f - h);

    float3 color = getColor(float3(d1.x,d1.y,d1.z), float3(d2.x,d2.y,d2.z), d1.w, d2.w);

    return to_float4(color, d);
}

float4 smoothSubtraction(float4 d1, float4 d2, float k){
    float4 negDistance = to_float4(float3(1.0f), -1.0f);
    return smoothUnion(d1, d2 * negDistance, k) * negDistance;
}

float4 smoothIntersection(float4 d1, float4 d2, float k){
    float4 negDistance = to_float4(float3(1.0f), -1.0f);
    return smoothUnion(d1 * negDistance, d2 * negDistance, k) * negDistance;
}

float4 differenceSDF(float4 a, float4 b) {
    return a.w > -b.w ? a : to_float4(float3(b.x, b.y, b.z), -b.w);
}

float3 getPos(float3 p, float3 offset) { return p - offset; }

float3 max(float3 q, float f){
    return max(q, float3(f));
    return float3(max(q.x, f), max(q.y, f), max(q.z, f));
}

float3 mod(float3 q, float f){
    return float3(mod(q.x, f), mod(q.y, f), mod(q.z,f));
}

float4 sdf_sphere(float3 p, float3 offset, float3 color, float r) {
    p = getPos(p, offset);
    float obj = length(p) - r;
    return to_float4(color, obj);
}

float4 sdf_plane(float3 p, float3 color, float3 n, float h, bool isCelled) {

    float plane = dot(p, normalize(n)) + h;

    if (isCelled) {
        color = float3(0.2f + 0.4f * mod(floor(p.x) + floor(p.z), 2.0f));
    }

    return to_float4(color, plane);
}

float4 sdf_box(float3 p, float3 offset, float3 color, float3 side){
    p = getPos(p, offset);
    float3 q = abs(p) - side;

    float box = length(max(q, 0.0f)) + min(max(q.x, max(q.y, q.z)), 0.0f);

    return to_float4(color, box);
}

float4 sdf_roundBox(float3 p, float3 offset, float3 color, float3 side, float r) {
    p = getPos(p, offset);
    float3 q = abs(p) - side + r;

    float box = length(max(q, 0.0f)) + min(max(q.x, max(q.y, q.z)), 0.0f) - r;

    return to_float4(color, box);
}

float4 sdf_mengerSponge(float3 p, float3 offset, float3 color, float3 side, float scale) {
    p = getPos(p, offset);
    p /= scale;
    float4 box = sdf_box(p, float3(0.0f), color, side);

    float s = 1.0f;

    for (int m = 0; m < 5; m++) {
        float3 a = mod((p) * s, 2.0f) - 1.0f;
        s *= 3.0;
        float3 r = abs(1.0f - 3.0f * abs(a));

        float da = max(r.x, r.y);
        float db = max(r.y, r.z);
        float dc = max(r.z, r.x);

        float c = (min(da, min(db, dc)) - 1.0f) / s;

        box.w = max(box.w, c);
    }

    return box;
}

float4 sdf_mandelbulb(float3 p, float3 offset, float3 color, int power, float scale) {
    p = getPos(p, offset);
    p /= scale;
    float3 z = p;

    float dr = 1;
    float r = 0;
    float bailout = 50.0f;

    for (int i = 0; i < 5; i++) {
        r = length(z);
        if (r > bailout) break;

        // convert to polar coordinates
        float theta = acos(z.z / r);
        float phi = atan2(z.y, z.x);
        dr = pow(r, power - 1.0f) * power * dr + 1.0f;

        // scale and rotate the point
        float zr = (float)pow(r, power);
        theta = theta * power;
        phi = phi * power;

        // convert back to cartesian coordinates
        z = zr * float3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
        z += (p) ;
    }

    return to_float4(color, 0.5f * log(r) * r / dr);
}