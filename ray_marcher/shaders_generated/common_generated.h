/////////////////////////////////////////////////////////////////////
/////////////  Required  Shader Features ////////////////////////////
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
/////////////////// include files ///////////////////////////////////
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
/////////////////// declarations in class ///////////////////////////
/////////////////////////////////////////////////////////////////////
#ifndef uint32_t
#define uint32_t uint
#endif
#define FLT_MAX 1e37f
#define FLT_MIN -1e37f
#define FLT_EPSILON 1e-6f
#define DEG_TO_RAD  0.017453293f
#define unmasked
#define half  float16_t
#define half2 f16vec2
#define half3 f16vec3
#define half4 f16vec4
bool  isfinite(float x)            { return !isinf(x); }
float copysign(float mag, float s) { return abs(mag)*sign(s); }

struct complex
{
  float re, im;
};

complex make_complex(float re, float im) { 
  complex res;
  res.re = re;
  res.im = im;
  return res;
}

complex to_complex(float re)              { return make_complex(re, 0.0f);}
complex complex_add(complex a, complex b) { return make_complex(a.re + b.re, a.im + b.im); }
complex complex_sub(complex a, complex b) { return make_complex(a.re - b.re, a.im - b.im); }
complex complex_mul(complex a, complex b) { return make_complex(a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re); }
complex complex_div(complex a, complex b) {
  const float scale = 1 / (b.re * b.re + b.im * b.im);
  return make_complex(scale * (a.re * b.re + a.im * b.im), scale * (a.im * b.re - a.re * b.im));
}

complex real_add_complex(float value, complex z) { return complex_add(to_complex(value),z); }
complex real_sub_complex(float value, complex z) { return complex_sub(to_complex(value),z); }
complex real_mul_complex(float value, complex z) { return complex_mul(to_complex(value),z); }
complex real_div_complex(float value, complex z) { return complex_div(to_complex(value),z); }

complex complex_add_real(complex z, float value) { return complex_add(z, to_complex(value)); }
complex complex_sub_real(complex z, float value) { return complex_sub(z, to_complex(value)); }
complex complex_mul_real(complex z, float value) { return complex_mul(z, to_complex(value)); }
complex complex_div_real(complex z, float value) { return complex_div(z, to_complex(value)); }

float real(complex z) { return z.re;}
float imag(complex z) { return z.im; }
float complex_norm(complex z) { return z.re * z.re + z.im * z.im; }
float complex_abs(complex z) { return sqrt(complex_norm(z)); }
complex complex_sqrt(complex z) 
{
  float n = complex_abs(z);
  float t1 = sqrt(0.5f * (n + abs(z.re)));
  float t2 = 0.5f * z.im / t1;
  if (n == 0.0f)
    return to_complex(0.0f);
  if (z.re >= 0.0f)
    return make_complex(t1, t2);
  else
    return make_complex(abs(t2), copysign(t1, z.im));
}


#ifndef SKIP_UBO_INCLUDE
#include "include/RayMarcher_generated_ubo.h"
#endif

/////////////////////////////////////////////////////////////////////
/////////////////// local functions /////////////////////////////////
/////////////////////////////////////////////////////////////////////

mat4 translate4x4(vec3 delta)
{
  return mat4(vec4(1.0, 0.0, 0.0, 0.0),
              vec4(0.0, 1.0, 0.0, 0.0),
              vec4(0.0, 0.0, 1.0, 0.0),
              vec4(delta, 1.0));
}

mat4 rotate4x4X(float phi)
{
  return mat4(vec4(1.0f, 0.0f,  0.0f,           0.0f),
              vec4(0.0f, +cos(phi),  +sin(phi), 0.0f),
              vec4(0.0f, -sin(phi),  +cos(phi), 0.0f),
              vec4(0.0f, 0.0f,       0.0f,      1.0f));
}

mat4 rotate4x4Y(float phi)
{
  return mat4(vec4(+cos(phi), 0.0f, -sin(phi), 0.0f),
              vec4(0.0f,      1.0f, 0.0f,      0.0f),
              vec4(+sin(phi), 0.0f, +cos(phi), 0.0f),
              vec4(0.0f,      0.0f, 0.0f,      1.0f));
}

mat4 rotate4x4Z(float phi)
{
  return mat4(vec4(+cos(phi), sin(phi), 0.0f, 0.0f),
              vec4(-sin(phi), cos(phi), 0.0f, 0.0f),
              vec4(0.0f,      0.0f,     1.0f, 0.0f),
              vec4(0.0f,      0.0f,     0.0f, 1.0f));
}

mat4 inverse4x4(mat4 m) { return inverse(m); }
vec3 mul4x3(mat4 m, vec3 v) { return (m*vec4(v, 1.0f)).xyz; }
vec3 mul3x3(mat4 m, vec3 v) { return (m*vec4(v, 0.0f)).xyz; }

mat3 make_float3x3(vec3 a, vec3 b, vec3 c) { // different way than mat3(a,b,c)
  return mat3(a.x, b.x, c.x,
              a.y, b.y, c.y,
              a.z, b.z, c.z);
}

vec3 getColor(vec3 c1, vec3 c2, float d1, float d2) {
    return (d1 * c2 + d2 * c1) / (d1 + d2);
}

vec3 getPos(vec3 p, vec3 offset) { return p - offset; }

// vec3 max(vec3 q, float f) {
//     return max(q, vec3(f));
//     return vec3(max(q.x, f),max(q.y, f),max(q.z, f));
// }

// vec3 mod(vec3 q, float f) {
//     return vec3(mod(q.x, f),mod(q.y, f),mod(q.z,f));
// }

vec4 smoothUnion(vec4 d1, vec4 d2, float k) {
    float h = clamp(0.5f + 0.5f * (d1.w - d2.w) / k, 0.0f, 1.0f);
    float d = mix(d1.w, d2.w, h) - k * h * (1.0f - h);

    vec3 color = getColor(vec3(d1.x,d1.y,d1.z), vec3(d2.x,d2.y,d2.z), d1.w, d2.w);

    return vec4(color, d);
}

vec4 sdf_box(vec3 p, vec3 offset, vec3 color, vec3 side) {
    p = getPos(p, offset);
    vec3 q = abs(p) - side;

    float box = length(max(q, 0.0f)) + min(max(q.x, max(q.y, q.z)), 0.0f);

    return vec4(color, box);
}

vec4 sdf_sphere(vec3 p, vec3 offset, vec3 color, float r) {
    p = getPos(p, offset);
    float obj = length(p) - r;
    return vec4(color, obj);
}

vec4 sdf_roundBox(vec3 p, vec3 offset, vec3 color, vec3 side, float r) {
    p = getPos(p, offset);
    vec3 q = abs(p) - side + r;

    float box = length(max(q, 0.0f)) + min(max(q.x, max(q.y, q.z)), 0.0f) - r;

    return vec4(color, box);
}

vec4 sdf_mandelbulb(vec3 p, vec3 offset, vec3 color, int power, float scale) {
    p = getPos(p, offset);
    p /= scale;
    vec3 z = p;

    float dr = 1;
    float r = 0;
    float bailout = 50.0f;

    for (int i = 0; i < 5; i++) {
        r = length(z);
        if (r > bailout) break;

        // convert to polar coordinates
        float theta = acos(z.z / r);
        float phi = atan(z.y,z.x);
        dr = pow(r, float(power) - 1.0f) * float(power) * dr + 1.0f;

        // scale and rotate the point
        float zr = float(pow(r, float(power)));
        theta = theta * float(power);
        phi = phi * float(power);

        // convert back to cartesian coordinates
        z = zr * vec3(sin(theta) * cos(phi),sin(phi) * sin(theta),cos(theta));
        z += (p) ;
    }

    return vec4(color, 0.5f * log(r) * r / dr);
}

vec4 sdf_plane(vec3 p, vec3 color, vec3 n, float h, bool isCelled) {

    float plane = dot(p, normalize(n)) + h;

    if (isCelled) {
        color = vec3(0.2f + 0.4f * mod(floor(p.x) + floor(p.z), 2.0f));
    }

    return vec4(color, plane);
}

vec4 sdf_mengerSponge(vec3 p, vec3 offset, vec3 color, vec3 side, float scale) {
    p = getPos(p, offset);
    p /= scale;
    vec4 box = sdf_box(p, vec3(0.0f), color, side);

    float s = 1.0f;

    for (int m = 0; m < 5; m++) {
        vec3 a = mod((p) * s, 2.0f) - 1.0f;
        s *= 3.0;
        vec3 r = abs(1.0f - 3.0f * abs(a));

        float da = max(r.x, r.y);
        float db = max(r.y, r.z);
        float dc = max(r.z, r.x);

        float c = (min(da, min(db, dc)) - 1.0f) / s;

        box.w = max(box.w, c);
    }

    return box;
}

vec4 smoothSubtraction(vec4 d1, vec4 d2, float k) {
    vec4 negDistance = vec4(vec3(1.0f), -1.0f);
    return smoothUnion(d1, d2 * negDistance, k) * negDistance;
}

vec4 unionSDF(vec4 a, vec4 b) {
    return a.w < b.w ? a : b;
}

vec4 map(vec3 p) {
    vec4 scene = vec4(1.0f);

    vec4 sphere = sdf_sphere(p, vec3(0.0f,2.0f,-3.0f), vec3(0.0f,0.0f,1.0f), 2.0f);
    vec4 sphere2 = sdf_sphere(p, vec3(0.0f,0.0f,0.0f), vec3(0.67f,0.87f,0.87f), 5.7f);
    vec4 sphere3 = sdf_sphere(p, vec3(0.0f,-1.5f,-5.0f), vec3(0.67f,0.87f,0.87f), 2.0f);

    vec4 rbox = sdf_roundBox(p, vec3(0.0f,-1.0f,0.0f), vec3(0.25f,0.79f,0.87f), vec3(5.0f), 0.5f);

    vec4 plain = sdf_plane(p, vec3(1.0f,1.0f,1.0f), vec3(0.0f,1.0f,0.0f), 1.5f, true);

    vec4 sponge = sdf_mengerSponge(p, vec3(0.0f,1.0f,0.0f), vec3(0.0f,0.55f,0.85f), vec3(1.0f), 2.0f);

    vec4 mandelbulb = sdf_mandelbulb(p, vec3(0.0f,7.0f,0.0f), vec3(0.25f,0.79f,0.87f), 7, 3.0f);

    scene = smoothSubtraction(sphere, sponge, 0.8f);

    vec4 tmp = smoothSubtraction(sphere2, rbox, 0.95f);
    scene = unionSDF(tmp, scene);

    tmp = smoothUnion(plain, sphere3, 0.9f);
    scene = unionSDF(scene, tmp);

    scene = unionSDF(mandelbulb, scene);

    return scene;
}

vec3 EstimateNormal(vec3 z, float eps) {
    vec3 z1 = z + vec3(eps,0,0);
    vec3 z2 = z - vec3(eps,0,0);
    vec3 z3 = z + vec3(0,eps,0);
    vec3 z4 = z - vec3(0,eps,0);
    vec3 z5 = z + vec3(0,0,eps);
    vec3 z6 = z - vec3(0,0,eps);
    float dx = map(z1).w - map(z2).w;
    float dy = map(z3).w - map(z4).w;
    float dz = map(z5).w - map(z6).w;
    return normalize(vec3(dx,dy,dz));
}

float getAmbientOcclusion(vec3 p, vec3 normal) {
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

float getSoftShadow(vec3 p, vec3 lightPos) {
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

mat3 getCam(vec3 ro, vec3 lookAt) {
	vec3 camF = normalize(lookAt - ro);
	vec3 camR = normalize(cross(vec3(0.0f,1.0f,0.0f), camF));
	vec3 camU = cross(camF, camR);

	return mat3(camR, camU, camF);
}

vec3 getLight(vec3 pos, vec3 rd, vec3 color) {
    vec3 lightPos = vec3(2.0f,20.0f,-5.0f);
    vec3 L = normalize(lightPos - pos);
    vec3 N = EstimateNormal(pos, 1e-3);
    vec3 V = -1 * rd;
    vec3 R = reflect(-1 * L, N);

    vec3 specColor = vec3(0.5f);
    vec3 specular = 1.3f * specColor * pow(clamp(dot(R, V), 0.0f, 1.0f), 10.0f);
    vec3 ambient = 0.05f * color;
    vec3 fresnel = 0.15f * color * pow(1.0f + dot(rd, N), 3.0f);

    float shadow = getSoftShadow(pos + N * 0.02f, normalize(lightPos));
    float occ = getAmbientOcclusion(pos, N);

    vec3 dif = color * clamp(dot(N, L), 0.1f, 1.0f);

    return (ambient + fresnel) * occ + (dif + specular * occ) * shadow;
}

vec2 getUV(vec2 offset, int x, int y, int width, int heigth) {
    float ratio = float(width) / float(heigth);
    return ((vec2(x,y) - offset) * 2.0f / vec2(width,heigth) - 1.0f) * vec2(ratio,1.0f);
}

uint RealColorToUint32(vec4 real_color) {
    int red = max(0, min(255, int((real_color[0] * 255.0f))));
    int green = max(0, min(255, int((real_color[1] * 255.0f))));
    int blue = max(0, min(255, int((real_color[2] * 255.0f))));
    int alpha = max(0, min(255, int((real_color[3] * 255.0f))));

    return red | (green << 8) | (blue << 16) | (alpha << 24);
}

#define KGEN_FLAG_RETURN            1
#define KGEN_FLAG_BREAK             2
#define KGEN_FLAG_DONT_SET_EXIT     4
#define KGEN_FLAG_SET_EXIT_NEGATIVE 8
#define KGEN_REDUCTION_LAST_STEP    16
#define MAXFLOAT FLT_MAX
#define CFLOAT_GUARDIAN 

