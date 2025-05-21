#pragma once
#ifndef WATER_FFT_H
#define WATER_FFT_H

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <vector>
#include <cmath>
#include <map>

#include "GUI.h"
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

    bool hasCreateDebugPlane;
    static const int DEBUGSIZE = 5;
    static const int TEXTURE_ARRAY_SIZE = 4;

    struct attributeArray {
        unsigned int vao, ebo;
        unsigned int indicesSize;
        unsigned int tex;
    } patchAttribute, debug[DEBUGSIZE * TEXTURE_ARRAY_SIZE];

    struct SpectrumSettings {
        float scale;
        float angle;
        float spreadBlend;
        float swell;
        float alpha;
        float peakOmega;
        float gamma;
        float shortWavesFade;
        
        bool useSpectrum;
        float tile;
    } spectrumParam[TEXTURE_ARRAY_SIZE];

    GUI::WaterFFTParam waterFFTParam;

    unsigned int initialSpectrumTexture, spectrumTexture, derivativeTexture;
    unsigned int displacementTexture, slopeTexture;

    ShaderT *waterFFTShader;
    ComputeShader* compute_InitialSpectrum;
    ComputeShader* compute_PackSpectrumConjugate;
    ComputeShader* compute_UpdateSpectrum;
    ComputeShader* compute_FFTHorizontal;
    ComputeShader* compute_FFTVertical;
    ComputeShader* compute_AssembleMaps;

    WaterFFT();
    ~WaterFFT();

    void createPlane();
    void createShader(std::string filename);
    void createComputeShader();
    unsigned int createRenderTexture(int binding, int arrayTextureSize);

    void initTexture();
    void initializeSpectrum();
    void initUniform();

    void inverseFFT();
    void updateSpectrumToFFT(float frameTime);

    void update(float frameTime);
    void draw(glm::mat4 projection, glm::mat4 view, glm::vec3 viewPos, unsigned int environmentMap);

    void createDebugPlane();
    void drawDebugPlane(Shader* shader, glm::mat4 projection, glm::mat4 view);

    void setPlaneSize(int n);
    void setTextureSize(int n);
    void setPos(glm::vec3 p);

private:
    void updateSpectrumParam();
    float JonswapAlpha(float fetch, float windSpeed);
    float JonswapPeakFrequency(float fetch, float windSpeed);
};

#endif // !WATER_FFT_H
