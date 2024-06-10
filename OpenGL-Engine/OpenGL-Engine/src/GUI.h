#pragma once
#ifndef GUI_H
#define GUI_H

#include <glm/glm.hpp>

#include "imGui/imgui.h"
#include "imGui/imgui_impl_glfw.h"
#include "imGui/imgui_impl_opengl3.h"

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

    void initialize(GLFWwindow* w);
    void GUIFrame();
    void modelTransform(glm::vec3& pos, glm::vec3& rot, float& angle, glm::vec3& scale);
    // later;
    void modelAnimation(int animation, int n);
    bool proceduralTerrainParam(int& seed, float& scale, int& octaves, float& persistence, float& lacunarity, glm::vec2& offset, float& heightMultiplier);
    void grassParam(float& frequency, float& amplitude, float& scale, float& drop);

    //water
    void waterParam(float& a, float& f, float& s, int &waveCount);
    void fogDistanceParam(float& pNear, float& pFar, float& pDensity);
    void renderUI();
    void shutDown();
};


#endif