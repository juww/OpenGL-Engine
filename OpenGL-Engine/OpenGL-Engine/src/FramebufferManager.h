#pragma once
#ifndef FRAMEBUFFER_MANAGER_H
#define FRAMEBUFFER_MANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <vector>
#include <map>

#include "interpolate.h"
#include "shader_m.h"
#include "renderObject.h"

class FramebufferManager {
public:
    FramebufferManager();
    ~FramebufferManager();
    static FramebufferManager* getInstance();
    int m_ScreenWidth, m_ScreenHeight;
    unsigned int m_Fbo;

    Shader* deferredShader;

    Shader* GBufferShader;
    unsigned int gBuffer;
    unsigned int gPosition, gNormal, gAlbedo, gNormalMap, gORMMap, gDepth;
    unsigned int gViewPosition, gViewNormal, gTexCoords;

    unsigned int gRbo;

    Shader* SSAOShader;
    Shader* SSAOBlurShader;
    std::vector<glm::vec3> ssaoKernel;
    std::vector<glm::vec3> ssaoNoise;
    float ssaoRadius, ssaoBias;
    unsigned int ssaoFBO, ssaoBlurFBO;
    unsigned int ssaoBuffer, ssaoBufferBlur;
    unsigned int noiseTexture;

    unsigned int captureFBO;
    unsigned int captureRBO;
    unsigned int irradianceMap;
    unsigned int preFilterMap;
    unsigned int brdfLUTTexture;
    std::map<std::string, unsigned int> mappers;

    std::map<unsigned int, RenderObject> objects;

    void setScreenSpace();
    void createDepthStencilFramebuffer();
    void IrradianceMapping(Shader* irradianceShader, unsigned int envCubemap, int width, int height);
    void PreFilterMapping(Shader* preFilterShader, unsigned int envCubemap, int width, int height);
    void BrdfLUT(Shader* LUTShader, int width, int height);
    void shaderConfig(Shader* shader);

    void generateGBuffer();
    void copyRenderObjects(std::map<unsigned int, RenderObject>& pRenderObject);
    void setGeometryPassShader(Shader* p_GBufferShader);
    void drawGBuffer(glm::mat4 projection, glm::mat4 view);
    void genScreenSpaceAmbientOcclusion();
    void setSSAOShader(Shader* p_SSAOShader, Shader* p_SSAOBlurShader);
    void drawSSAO(glm::mat4 projection, glm::mat4 view);
    void SSAOBlur();

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
    
    unsigned int quadVAO = 0;
    unsigned int quadVBO = 0;

    unsigned int cubeVAO = 0;
    unsigned int cubeVBO = 0;

    const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    const glm::mat4 captureViews[6] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

};

#endif // !FRAMEBUFFER_MANAGER_H
