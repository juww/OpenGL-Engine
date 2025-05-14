#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout (rgba32f, binding = 1) uniform image2D spectrumTexture;
layout (rgba32f, binding = 2) uniform image2D derivativeTexture;
layout (rgba32f, binding = 3) uniform image2D displacementTexture;
layout (rgba32f, binding = 4) uniform image2D slopeTexture;

#define PI 3.14159265359

uniform int _N;
uniform vec2 _Lambda;

vec2 ComplexMult(vec2 a, vec2 b) {
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

vec2 EulerFormula(float x) {
    return vec2(cos(x), sin(x));
}

float rcp(float a){
    return 1.0f / a;
}

vec4 Permute(vec4 data, vec2 id) {
    float mod2 = 2.0f;
    return data * (1.0f - 2.0f * mod((id.x + id.y), mod2));
}

void main() {

    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    vec2 idf = vec2(gl_GlobalInvocationID.xy);


    for (int i = 0; i < 1; ++i) {

        vec4 htildeDisplacement = Permute(imageLoad(spectrumTexture, id), idf);
        vec4 htildeSlope = Permute(imageLoad(derivativeTexture, id), idf);

        vec2 dxdz = htildeDisplacement.rg;
        vec2 dydxz = htildeDisplacement.ba;
        vec2 dyxdyz = htildeSlope.rg;
        vec2 dxxdzz = htildeSlope.ba;
        
        float jacobian = (1.0f + _Lambda.x * dxxdzz.x) * (1.0f + _Lambda.y * dxxdzz.y) - _Lambda.x * _Lambda.y * dydxz.y * dydxz.y;

        vec3 displacement = vec3(_Lambda.x * dxdz.x, dydxz.x, _Lambda.y * dxdz.y);

        vec2 slopes = dyxdyz.xy / (1.0 + abs(dxxdzz * _Lambda));
        float covariance = slopes.x * slopes.y;

        // float foam = _DisplacementTextures[uint3(id.xy, i)].a;
        // foam *= exp(-_FoamDecayRate);
        // foam = saturate(foam);

        // float biasedJacobian = max(0.0f, -(jacobian - _FoamBias));

        // if (biasedJacobian > _FoamThreshold) foam += _FoamAdd * biasedJacobian;

        // if (i == 0) _BuoyancyData[id.xy] = displacement.y;

        //_DisplacementTextures[uint3(id.xy, i)] = float4(displacement, foam);
        //_SlopeTextures[uint3(id.xy, i)] = float2(slopes);

        vec4 displacementValue = vec4(displacement, 0.0f);
        vec4 slopeValue = vec4(slopes, 0.0f, 0.0f);

        imageStore(displacementTexture, id, displacementValue);
        imageStore(slopeTexture, id, slopeValue);
    }

}