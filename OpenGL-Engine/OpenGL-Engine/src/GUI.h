#pragma once
#ifndef GUI_H
#define GUI_H

#include <glm/glm.hpp>
#include <string>

#include "imGui/imgui.h"
#include "imGui/imgui_impl_glfw.h"
#include "imGui/imgui_impl_opengl3.h"

#include "animator.h"

namespace GUI {

    // procedural Terrain param
    class TerrainParam {
    public:
        int m_Seed;
        float m_Scale;
        int m_Octaves;
        float m_Persistence;
        float m_Lacunarity;
        glm::vec2 m_OffsetV;
        float m_Offset[2];
        float m_Amplitude;
        TerrainParam(const int& seed, const float &scale, const int &octaves, const float &persistence, const float &lacunarity, const glm::vec2 offset, const float& amplitude) :
            m_Seed(seed), m_Scale(scale), m_Octaves(octaves), m_Persistence(persistence), m_Lacunarity(lacunarity), m_OffsetV(offset), m_Amplitude(amplitude){
            m_Offset[0] = offset[0];
            m_Offset[1] = offset[1];
        }

        ~TerrainParam(){}
    };

    class GrassParam {
    public:
        float m_Frequency = 3.0f;
        float m_Amplitude = 0.5f;
        float m_Scale = 1.12f;
        float m_Drop = 0.7f;
        GrassParam(const float& f, const float& a, const float& s, const float& d) {
            m_Frequency = f;
            m_Amplitude = a;
            m_Scale = s;
            m_Drop = d;
        }
    };
    
    class WaterParam {
    public:
        float m_Frequency = 1.0f;
        float m_Amplitude = 1.0f;
        float m_Speed = 1.0f;
        float m_Seed = 1;
        float m_SeedIter = 1234.0f;
        int m_WaveCount = 1;

        WaterParam(const float &f, const float &a, const float &s, const float &seed, const float &iter, const int& waveCount) {
            m_Frequency = f;
            m_Amplitude = a;
            m_Speed = s;
            m_Seed = seed;
            m_SeedIter = iter;
            m_WaveCount = waveCount;
        }
    };

    class WaterFFTParam {
    public:
        struct SpectrumParam {
            float lengthScale;
            float tile;

            float scale;
            float windSpeed;
            float windDirection;
            float fetch;
            float spreadBlend;
            float swell;
            float peakEnhancement;
            float shortWavesFade;

            bool useSpectrum;

        } spectrumParam[4];

        struct WaterUniform {
            int arrayTextureSize = 1;
            int seed = 1234;
            float lightDirection[3] = { 0.0f, 1.0f, 0.0f };
            float lowCutoff = 0.0001f;
            float highCutoff = 9000.0f;
            float gravity = 9.81f;
            float depth = 20.0f;
            float repeatTime = 200.0f;
            float speed = 1.0f;
            float lambda[2] = { 1.0f, 1.0f };
            float displacementDepthFalloff = 1.0f;
            float normalStrength = 1.0f;
            float normalDepthFalloff = 1.0f;
        } waterUniform;

        struct PBRWaterParam {
            float roughness;
            float metallic;
            float sunIrradiance[3];
            float scatterColor[3];
            float bubbleColor[3];
            float heightModifier;
            float foamRoughnessModifier;
            float bubbleDensity;
            float wavePeakScatterStrength;
            float scatterStrength;
            float scatterShadowStrength;
            float environmentLightStrength;
        } PBRWater;

        struct FoamParam {
            float foamColor[3];
            float foamBias;
            float foamDecayRate;
            float foamAdd;
            float foamThreshold;
            
            float foamSubtract[4];
        } foamParam;
    };

    class FogDistanceParam {
    public:
        float m_Near = 0.0f;
        float m_Far = 0.0f;
        float m_Density = 0.0f;
        glm::vec3 m_Color = glm::vec3(1.0f);
        FogDistanceParam(const float& pNear, const float& pFar, const float& pDensity, const glm::vec3 &pColor) {
            m_Near = pNear;
            m_Far = pFar;
            m_Density = pDensity;
            m_Color = pColor;
        }
    };
    
    class PBRParam {
    public:
        glm::vec4 m_BaseColor = glm::vec4(1.0f);
        float m_RoughnessFactor = 0.5f;
        float m_SubSurface = 0.5f;
        float m_MetallicFactor = 0.0f;

        float m_Specular = 0.5f;
        float m_SpecularTint = 1.0f;
        float m_Sheen = 0.0f;
        float m_SheenTint = 0.5f;
        float m_Anisotropic = 0.5f;
        float m_ClearCoatGloss = 0.5f;
        float m_ClearCoat = 0.5f;

        float m_HeightScale = 0.05f;

        PBRParam() {

        }
    };

    class DeferredParam {
    public:
        bool useDeferredRender = false;
        bool useSSAO = true;
        float ssaoRadius = 0.5f, ssaoBias = 0.005f;
    };

    void initialize(GLFWwindow* w);
    void GUIFrame();
    void modelTransform(std::string name, glm::vec3& pos, glm::vec3& rot, float& angle, glm::vec3& scale);
    void modelAnimation(std::string name, Animator& animator, bool &playAnimation);
    void PBRWindow(PBRParam &pbr);
    bool proceduralTerrainParam(int& seed, float& scale, int& octaves, float& persistence, float& lacunarity, glm::vec2& offset, float& heightMultiplier);
    bool lightSunParam(glm::vec3 &lightDirection, unsigned int depthMap);
    void grassParam(GrassParam& gp);
    void waterFFTParam(WaterFFTParam& waterFFTParams);
    void showTextureGBuffer(std::map<unsigned int, std::string>& p_GBuffer);
    void useDeferredShading(DeferredParam& deferredParam);
    void color01(float temp[], int n);
    glm::vec3 vecColor3(float temp[]);

    void waterParam(WaterParam& wp);
    void fogDistanceParam(FogDistanceParam& fp);
    void renderUI();
    void shutDown();
};


#endif