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
    

    void initialize(GLFWwindow* w);
    void GUIFrame();
    void modelTransform(glm::vec3& pos, glm::vec3& rot, float& angle, glm::vec3& scale);
    // later;
    void modelAnimation(int animation, int n);
    bool proceduralTerrainParam(int& seed, float& scale, int& octaves, float& persistence, float& lacunarity, glm::vec2& offset, float& heightMultiplier);
    void grassParam(float& frequency, float& amplitude, float& scale, float& drop);
    void renderUI();
    void shutDown();
};


#endif