#pragma once
#ifndef WATER_FFT_H
#define WATER_FFT_H

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <vector>
#include <cmath>

#include "interpolate.h"
#include "shader_m.h"

class WaterFFT {
public:

    int N;
    int logN;
    unsigned int m_Seed;
    float m_LowCutoff, m_HighCutoff;
    float m_Gravity, m_Depth;

    float m_LengthScales[4];

    struct spectrumSettings {
        float scale;
        float angle;
        float spreadBlend;
        float swell;
        float alpha;
        float peakOmega;
        float gamma;
        float shortWavesFade;
    };

    struct spectrumParameter {
        float scale;
        float windDirection;
        float spreadBlend;
        float swell;
        float fetch;
        float windSpeed;
        float peakEnhancement;
        float shortWavesFade;
    };

    struct quadPlane {
        unsigned int vao, ebo;
        unsigned int tex;
    };
    std::vector<quadPlane> m_Quad;
    std::vector<glm::vec4> initSpectrumTextures[9];
    std::vector<glm::vec4> spectrumTextures[5], fftGroupBuffer[2];
    std::vector<glm::vec4> displacementTextures, slopeTextures;

    WaterFFT(int n);
    ~WaterFFT();
    void initialSpectrum();
    void conjugateSpectrum();
    void update(float frameTime);
    void createQuad(int lod, int k);
    void drawTexture(Shader* shader, glm::mat4 projection, glm::mat4 view);

private:
    float hash(unsigned int n);
    glm::vec2 ComplexMult(glm::vec2 a, glm::vec2 b);
    glm::vec2 EulerFormula(float x);
    glm::vec2 UniformToGaussian(float u1, float u2);
    float Dispersion(float kMag);
    float DispersionDerivative(float kMag);
    float TMACorrection(float omega);
    float JONSWAP(float omega, spectrumSettings spectrum);
    float DirectionSpectrum(float theta, float omega, spectrumSettings spectrum);
    float ShortWavesFade(float kLength, spectrumSettings spectrum);
    float Cosine2s(float theta, float s);
    float SpreadPower(float omega, float peakOmega);
    float NormalizationFactor(float s);
    float JonswapAlpha(float fetch, float windSpeed);
    float JonswapPeakFrequency(float fetch, float windSpeed);
    void updateSpectrum(float repeatTime, float frameTime);
    void horizontalFFT();
    void verticalFFT();
    void ButterflyValues(unsigned int step, unsigned int index, glm::ivec2& indices, glm::vec2& twiddle);
    glm::vec4 FFT(unsigned int Index, glm::vec4 input);
    glm::vec4 Permute(glm::vec4 data, glm::vec3 ids);
    void assembleMaps(glm::vec2 lambda, float foamDecayRate, float foamBias);
    void setSpectrumSettings(spectrumSettings& spectrumSetting, spectrumParameter& spectrumParameter);
};

#endif // !WATER_FFT_H
