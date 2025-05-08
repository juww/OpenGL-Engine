#pragma once
#ifndef WATER_FFT_H
#define WATER_FFT_H

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <vector>
#include <cmath>

#include "interpolate.h"
#include "shader_t.h"
#include "shader_m.h"
#include "computeShader.h"

class WaterFFT {
public:
    glm::vec3 pos;

    int planeSize;
    int textureSize;

    bool updateSpectrum;

    struct attributeArray {
        unsigned int vao, ebo;
        unsigned int indicesSize;
    } patchAttribute, debug[2];

    struct SpectrumSettings {
        float scale;
        float angle;
        float spreadBlend;
        float swell;
        float alpha;
        float peakOmega;
        float gamma;
        float shortWavesFade;
    } spectrumParam;

    struct DisplaySpectrumSettings {
        float scale;
        float windSpeed;
        float windDirection;
        float fetch;
        float spreadBlend;
        float swell;
        float peakEnhancement;
        float shortWavesFade;
    } displaySpectrum;

    struct WaterUniform {
        int seed;
        float lowCutoff;
        float highCutoff;
        float gravity;
        float depth;
        float repeatTime;
        float speed;
        glm::vec2 lambda;
        float displacementDepthFalloff;

        float normalStrength;
        float normalDepthFalloff;
    } waterUniform;

    unsigned int initialSpectrumTexture, spectrumTexture;

    ShaderT *waterShader;
    ComputeShader* compute_InitialSpectrum;
    ComputeShader* compute_PackSpectrumConjugate;
    ComputeShader* compute_UpdateSpectrum;

    WaterFFT();
    ~WaterFFT();

    void createPlane();
    void createShader(std::string filename);
    void createComputeShader();
    unsigned int createRenderTexture(int binding);

    void initializeSpectrum();
    void initUniform();
    void setUniform(ComputeShader* computeShader);

    void updateSpectrumToFFT(float frameTime);

    void update();
    void draw(glm::mat4 projection, glm::mat4 view);

    void createDebugPlane();
    void drawDebugPlane(Shader* shader, glm::mat4 projection, glm::mat4 view);

    void setPlaneSize(int n);
    void setTextureSize(int n);
    void setPos(glm::vec3 p);

private:
    float JonswapAlpha(float fetch, float windSpeed);
    float JonswapPeakFrequency(float fetch, float windSpeed);
};

#endif // !WATER_FFT_H
