#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout (rgba32f, binding = 0) uniform image2DArray initialSpectrum;

uniform int _N;
uniform int _ArrayTextureSize;

void main() {

    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    
    for (uint i = 0; i < _ArrayTextureSize; ++i) {
        ivec3 idx = ivec3(id, i);

        vec2 h0 = imageLoad(initialSpectrum, idx).rg;

        ivec3 idconj = ivec3((_N - id.x ) % _N, (_N - id.y) % _N, i);
        vec2 h0conj = imageLoad(initialSpectrum, idconj).rg;

        value = vec4(h0, h0conj.x, -h0conj.y);
        imageStore(initialSpectrum, idx, value);
    }

}