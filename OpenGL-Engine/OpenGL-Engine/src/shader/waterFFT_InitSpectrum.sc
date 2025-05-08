#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout (rgba32f, binding = 0) uniform image2D initialSpectrum;

#define PI 3.14159265359

uniform int _N;
uniform int _Seed;
uniform float _LowCutoff;
uniform float _HighCutoff;
uniform float _Gravity;
uniform float _Depth;
uniform int _LengthScale0;
uniform int _LengthScale1;
uniform int _LengthScale2;
uniform int _LengthScale3;

struct SpectrumParameters {
	float scale;
	float angle;
	float spreadBlend;
	float swell;
	float alpha;
	float peakOmega;
	float gamma;
	float shortWavesFade;
};

uniform SpectrumParameters _Spectrums[2];

vec2 ComplexMult(vec2 a, vec2 b) {
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

vec2 EulerFormula(float x) {
    return vec2(cos(x), sin(x));
}

float hash(uint n) {
    // integer hash copied from Hugo Elias
    n = (n << 13U) ^ n;
    n = n * (n * n * 15731U + 0x789221U) + 0xD5D20589U;
    return float(n & uint(0x7fffffffU)) / float(0x7fffffff);
}

vec2 UniformToGaussian(float u1, float u2) {
    float R = sqrt(-2.0f * log(u1));
    float theta = 2.0f * PI * u2;

    return vec2(R * cos(theta), R * sin(theta));
}

float Dispersion(float kMag) {
    return sqrt(_Gravity * kMag * tanh(min(kMag * _Depth, 20)));
}

float DispersionDerivative(float kMag) {
    float th = tanh(min(kMag * _Depth, 20));
    float ch = cosh(kMag * _Depth);
    return _Gravity * (_Depth * kMag / ch / ch + th) / Dispersion(kMag) / 2.0f;
}

float NormalizationFactor(float s) {
    float s2 = s * s;
    float s3 = s2 * s;
    float s4 = s3 * s;
    if (s < 5) return -0.000564f * s4 + 0.00776f * s3 - 0.044f * s2 + 0.192f * s + 0.163f;
    else return -4.80e-08f * s4 + 1.07e-05f * s3 - 9.53e-04f * s2 + 5.90e-02f * s + 3.93e-01f;
}

float DonelanBannerBeta(float x) {
	if (x < 0.95f) return 2.61f * pow(abs(x), 1.3f);
	if (x < 1.6f) return 2.28f * pow(abs(x), -1.3f);

	float p = -0.4f + 0.8393f * exp(-0.567f * log(x * x));
	return pow(10.0f, p);
}

float DonelanBanner(float theta, float omega, float peakOmega) {
	float beta = DonelanBannerBeta(omega / peakOmega);
	float sech = 1.0f / cosh(beta * theta);
	return beta / 2.0f / tanh(beta * 3.1416f) * sech * sech;
}

float Cosine2s(float theta, float s) {
	return NormalizationFactor(s) * pow(abs(cos(0.5f * theta)), 2.0f * s);
}

float SpreadPower(float omega, float peakOmega) {
	if (omega > peakOmega)
		return 9.77f * pow(abs(omega / peakOmega), -2.5f);
	else
		return 6.97f * pow(abs(omega / peakOmega), 5.0f);
}

float DirectionSpectrum(float theta, float omega, SpectrumParameters spectrum) {
	float s = SpreadPower(omega, spectrum.peakOmega) + 16 * tanh(min(omega / spectrum.peakOmega, 20)) * spectrum.swell * spectrum.swell;
	return mix(2.0f / 3.1415f * cos(theta) * cos(theta), Cosine2s(theta - spectrum.angle, s), spectrum.spreadBlend);
}

float TMACorrection(float omega) {
	float omegaH = omega * sqrt(_Depth / _Gravity);
	if (omegaH <= 1.0f)
		return 0.5f * omegaH * omegaH;
	if (omegaH < 2.0f)
		return 1.0f - 0.5f * (2.0f - omegaH) * (2.0f - omegaH);

	return 1.0f;
}

float JONSWAP(float omega, SpectrumParameters spectrum) {
	float sigma = (omega <= spectrum.peakOmega) ? 0.07f : 0.09f;

	float r = exp(-(omega - spectrum.peakOmega) * (omega - spectrum.peakOmega) / 2.0f / sigma / sigma / spectrum.peakOmega / spectrum.peakOmega);
	
	float oneOverOmega = 1.0f / omega;
	float peakOmegaOverOmega = spectrum.peakOmega / omega;
	return spectrum.scale * TMACorrection(omega) * spectrum.alpha * _Gravity * _Gravity
		* oneOverOmega * oneOverOmega * oneOverOmega * oneOverOmega * oneOverOmega
		* exp(-1.25f * peakOmegaOverOmega * peakOmegaOverOmega * peakOmegaOverOmega * peakOmegaOverOmega)
		* pow(abs(spectrum.gamma), r);
}

float ShortWavesFade(float kLength, SpectrumParameters spectrum) {
	return exp(-spectrum.shortWavesFade * spectrum.shortWavesFade * kLength * kLength);
}

void main() {

    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    
    uint seed = id.x + _N * id.y + _N;
    seed += _Seed;

    float lengthScales[4] = { _LengthScale0, _LengthScale1, _LengthScale2, _LengthScale3 };

    for(uint i = 0; i < 1; i++){
        float halfN = _N / 2.0;

        float deltaK = 2.0 * PI / lengthScales[i];
        vec2 K = (id.xy - halfN) * deltaK;
        float kLength = length(K);

        seed += i + uint(hash(seed)) * 10;
        vec4 uniformRandSamples = vec4(hash(seed), hash(seed * 2), hash(seed * 3), hash(seed * 4));
        vec2 gauss1 = UniformToGaussian(uniformRandSamples.x, uniformRandSamples.y);
        vec2 gauss2 = UniformToGaussian(uniformRandSamples.z, uniformRandSamples.w);

        if (_LowCutoff <= kLength && kLength <= _HighCutoff) {
            float kAngle = atan(K.y, K.x);
            float omega = Dispersion(kLength);

            float dOmegadk = DispersionDerivative(kLength);

            float spectrum = JONSWAP(omega, _Spectrums[i * 2]) * DirectionSpectrum(kAngle, omega, _Spectrums[i * 2]) * ShortWavesFade(kLength, _Spectrums[i * 2]);
            
            //if (_Spectrums[i * 2 + 1].scale > 0)
            //    spectrum += JONSWAP(omega, _Spectrums[i * 2 + 1]) * DirectionSpectrum(kAngle, omega, _Spectrums[i * 2 + 1]) * ShortWavesFade(kLength, _Spectrums[i * 2 + 1]);
            
            //_InitialSpectrumTextures[uint3(id.xy, i)] = float4(float2(gauss2.x, gauss1.y) * sqrt(2 * spectrum * abs(dOmegadk) / kLength * deltaK * deltaK), 0.0f, 0.0f);
            value = vec4(vec2(gauss2.x, gauss1.y) * sqrt(2 * spectrum * abs(dOmegadk) / kLength * deltaK * deltaK), 0.0f, 0.0f);
            //value = vec4(vec3(gauss2.x, gauss1.y, 0.0f), 1.0f);
        } else {
            //_InitialSpectrumTextures[uint3(id.xy, i)] = 0.0f;
            value = vec4(0.0f);
        }
    }

    imageStore(initialSpectrum, id, value);
}