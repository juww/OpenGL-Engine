#pragma once
#ifndef FRAMEBUFFER_MANAGER_H
#define FRAMEBUFFER_MANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <vector>
#include <map>

#include "shader_m.h"

class FramebufferManager {
public:
    FramebufferManager();
    ~FramebufferManager();
    static FramebufferManager* getInstance();
    int m_ScreenWidth, m_ScreenHeight;
    unsigned int m_Fbo;

    unsigned int gBuffer;
    unsigned int gPosition, gNormal, gColor;
    unsigned int gRboDepth;

    unsigned int captureFBO;
    unsigned int captureRBO;
    unsigned int irradianceMap;
    unsigned int preFilterMap;
    unsigned int brdfLUTTexture;
    std::map<std::string, unsigned int> mappers;

    void setScreenSpace();
    void createDepthStencilFramebuffer();
    void IrradianceMapping(Shader* irradianceShader, unsigned int envCubemap, int width, int height);
    void PreFilterMapping(Shader* preFilterShader, unsigned int envCubemap, int width, int height);
    void BrdfLUT(Shader* LUTShader, int width, int height);
    void shaderConfig(Shader* shader);

    void generateGBuffer();

    void bindFramebuffers();
    void setFogDistance(Shader* shader, float& near, float& far, float& density, glm::vec3& fogColor);
    void draw(Shader* shader);
    void deleteFramebuffer(unsigned int& fbo);
    unsigned int combineTexture(Shader* shader, std::map<std::string, unsigned int>& pTexture, int width, int height);

    void renderQuad();
private:
    
    void renderCube();

    std::vector<float> m_ScreenVertices;

    static FramebufferManager* instance;

    unsigned int m_Texture;
    unsigned int m_DepthTex;
    unsigned int m_Vao, m_Rbo;
    

    const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    const glm::mat4 captureViews[6] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

};

#endif // !FRAMEBUFFER_MANAGER_H
