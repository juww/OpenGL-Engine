#ifndef SHADOW_H
#define SHADOW_H

#include <glm/glm.hpp>
#include "shader_m.h"

class Shadow {
public:
    glm::vec3 lightDirection;
    glm::vec3 lightPosition;

    glm::mat4 lightProjection;
    glm::mat4 lightView;

    unsigned int width, height;
    float aspect;

    unsigned int depthFBO, depthMap;
    Shader* shadowMappingShader;

    Shadow();
    ~Shadow();

    void setShadowSizeScreen(unsigned int w, unsigned int h);
    void setLightView(const glm::vec3& lookAtPosition, const glm::vec3& up);
    void setProjectionOrtho(const glm::vec4& dimension, const float& near, const float& far);
    void setProjectionPerspective(const float& fow, const float& near, const float& far);

    void framebufferDepthMap();
    void renderDepthBuffer();

};

#endif
