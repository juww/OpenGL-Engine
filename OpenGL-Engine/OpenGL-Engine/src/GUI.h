#pragma once
#ifndef GUI_H
#define GUI_H

#include <glm/glm.hpp>

#include "imGui/imgui.h"
#include "imGui/imgui_impl_glfw.h"
#include "imGui/imgui_impl_opengl3.h"

namespace GUI {

    // procedural Terrain param
    class terrainParam {
    public:
        int pSeed;
        float pScale;
        int pOctaves;
        float pPersistence;
        float pLacunarity;
        glm::vec2 po;
        float pOffset[2];
        float pAmplitude;
        terrainParam(const int seed, const float &scale, const int &octaves, const float &persistence, const float &lacunarity, const glm::vec2 offset, const float& amplitude) :
            pSeed(seed), pScale(scale), pOctaves(octaves), pPersistence(persistence), pLacunarity(lacunarity), po(offset), pAmplitude(amplitude){
            pOffset[0] = offset[0];
            pOffset[1] = offset[1];
        }

        ~terrainParam(){}
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