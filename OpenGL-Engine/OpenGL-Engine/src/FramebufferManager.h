#pragma once
#ifndef FRAMEBUFFER_MANAGER_H
#define FRAMEBUFFER_MANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "shader_m.h"

class FramebufferManager {
public:
    FramebufferManager();
    ~FramebufferManager();
    static FramebufferManager* getInstance();
    int m_ScreenWidth, m_ScreenHeight;
    unsigned int m_Fbo;

    void setScreenSpace();
    void createDepthStencilFramebuffer();
    void shaderConfig(Shader* shader);
    void bindFramebuffers();
    void setFogDistance(Shader* shader, float& near, float& far, float& density, glm::vec3& fogColor);
    void draw(Shader* shader);
    void deleteFramebuffer(unsigned int& fbo);

private:
    
    std::vector<float> m_ScreenVertices;

    static FramebufferManager* instance;

    unsigned int m_Texture;
    unsigned int m_DepthTex;
    unsigned int m_Vao, m_Rbo;
};

#endif // !FRAMEBUFFER_MANAGER_H
