#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout (rgba32f, binding = 0) uniform image2D imgOutput;

//
// Description : Array and textureless GLSL 2D/3D/4D simplex
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20201014 (stegu)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
//

uniform int u_seed;
uniform float u_scale;
uniform int u_octaves;
uniform float u_persistence;
uniform float u_lacunarity;
uniform vec2 u_size;
uniform vec2 u_offset;

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+10.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

vec2 fade(vec2 t) {
  return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

vec2 DFade(vec2 t){
    return 30.0 * t * t * (t * (t - 2.0) + 1.0);
}

// Classic Perlin noise
vec3 cnoise(vec2 P) {

    vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
    vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
    Pi = mod289(Pi); // To avoid truncation effects in permutation
    vec4 ix = Pi.xzxz;
    vec4 iy = Pi.yyww;
    vec4 fx = Pf.xzxz;
    vec4 fy = Pf.yyww;

    vec4 i = permute(permute(ix) + iy);

    vec4 gx = fract(i * (1.0 / 41.0)) * 2.0 - 1.0 ;
    vec4 gy = abs(gx) - 0.5 ;
    vec4 tx = floor(gx + 0.5);
    gx = gx - tx;

    vec2 g00 = vec2(gx.x,gy.x); // ga
    vec2 g10 = vec2(gx.y,gy.y); // gb
    vec2 g01 = vec2(gx.z,gy.z); // gc
    vec2 g11 = vec2(gx.w,gy.w); // gd

    vec4 norm = taylorInvSqrt(vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));

    float n00 = norm.x * dot(g00, vec2(fx.x, fy.x));
    float n01 = norm.y * dot(g01, vec2(fx.z, fy.z));
    float n10 = norm.z * dot(g10, vec2(fx.y, fy.y));
    float n11 = norm.w * dot(g11, vec2(fx.w, fy.w));

    vec2 fade_xy = fade(Pf.xy);
    vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
    float n_xy = mix(n_x.x, n_x.y, fade_xy.y);

    vec2 DFade_xy = DFade(Pf.xy);
    vec2 Derivate = g00 + fade_xy.x * (g10 - g00) + fade_xy.y * (g01 - g00) + 
                    fade_xy.x * fade_xy.y * (g00 - g10 - g01 + g11) +
                    DFade_xy * (fade_xy.yx * (n00 - n10 - n01 + n11) + vec2(n10, n01) - n00);

    return vec3(2.3 * n_xy, Derivate.x, Derivate.y);
}

void main() {

    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 halfN = u_size / 2.0;

    vec2 curCoord = vec2(texelCoord) - halfN;

    vec3 total = vec3(0.0);
    float frequency = 1.0;
    float amplitude = 1.0;

    for(int i = 0; i < u_octaves; i++){
        float x = (curCoord.x + u_offset.x) / u_scale;
        float y = (curCoord.y + u_offset.y) / u_scale;
        vec2 coord = vec2(x,y) * frequency;

        vec3 pp = cnoise(coord);
        total += (amplitude * pp);

        amplitude *= u_persistence;
        frequency *= u_lacunarity;
    }
    vec4 value = vec4(0.5 + 0.5 * vec3(total), 1.0);

    imageStore(imgOutput, ivec2(gl_GlobalInvocationID.xy), value);
}