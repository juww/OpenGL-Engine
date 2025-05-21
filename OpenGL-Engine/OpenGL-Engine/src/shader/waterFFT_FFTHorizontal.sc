#version 430 core

#define SIZE 1024
#define LOG_SIZE 10

layout (local_size_x = SIZE, local_size_y = 1, local_size_z = 1) in;

layout (rgba32f, binding = 1) uniform image2DArray spectrumTexture;
layout (rgba32f, binding = 2) uniform image2DArray derivativeTexture;

uniform int _ArrayTextureSize;

shared vec4 fftGroupBuffer[2][SIZE];

vec2 ComplexMult(vec2 a, vec2 b) {
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

void sincos(float angle, out float a, out float b){
    a = sin(angle);
    b = cos(angle);
}

void ButterflyValues(uint step, uint index, out uvec2 indices, out vec2 twiddle) {
    const float twoPi = 6.28318530718;
    uint b = SIZE >> (step + 1);
    uint w = b * (index / b);
    uint i = (w + index) % SIZE;
    sincos(-twoPi / float(SIZE) * float(w), twiddle.y, twiddle.x);

    //This is what makes it the inverse FFT
    twiddle.y = -twiddle.y;
    indices = uvec2(i, i + b);
}

vec4 FFT(uint threadIndex, vec4 inputTarget) {
    fftGroupBuffer[0][threadIndex] = inputTarget;
    memoryBarrierShared();
    barrier();
    int flag = 0;

    #pragma unroll
    for (uint step = 0; step < LOG_SIZE; ++step) {
        uvec2 inputsIndices;
        vec2 twiddle;
        ButterflyValues(step, threadIndex, inputsIndices, twiddle);

        vec4 v = fftGroupBuffer[flag][inputsIndices.y];
        int notFlag = (flag == 0 ? 1 : 0);
        fftGroupBuffer[notFlag][threadIndex] = fftGroupBuffer[flag][inputsIndices.x] + vec4(ComplexMult(twiddle, v.xy), ComplexMult(twiddle, v.zw));

        flag = notFlag;
        memoryBarrierShared();
        barrier();
    }

    return fftGroupBuffer[flag][threadIndex];
}

void main() {

    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    
    for (int i = 0; i < _ArrayTextureSize; ++i) {
        ivec3 idx = ivec3(id, i);
        vec4 fourierTarget = imageLoad(spectrumTexture, idx);
        value = FFT(id.x, fourierTarget);

        vec4 DfFourierTarget = imageLoad(derivativeTexture, idx);
        vec4 dValue = FFT(id.x, DfFourierTarget);

        imageStore(spectrumTexture, idx, value);
        imageStore(derivativeTexture, idx, dValue);
    }
}