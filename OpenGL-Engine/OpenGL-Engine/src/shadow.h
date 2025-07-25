#ifndef SHADOW_H
#define SHADOW_H

#include <glm/glm.hpp>
#include <map>

#include "shader_m.h"
#include "renderObject.h"

class Shadow {
public:
    glm::vec3 lightDirection;
    glm::vec3 lightPoV;

    glm::mat4 lightProjection;
    glm::mat4 lightView;
    glm::mat4 lightSpaceMatrix;

    unsigned int width, height;
    float aspect;

    unsigned int depthFBO, depthMap;
    Shader* shadowMappingShader;

    std::map<unsigned int, RenderObject> objects;

    Shadow();
    ~Shadow();

    void setShadowSizeScreen(unsigned int w, unsigned int h);
    void setShader(Shader* shader);
    void setLightPoV(glm::vec3 lightDir, float distance, glm::vec3 LookAtPosition);
    void setLightView(glm::vec3 lookAtPosition, const glm::vec3& up);
    void setProjectionOrtho(const glm::vec4& dimension, const float& near, const float& far);
    void setProjectionPerspective(const float& fow, const float& near, const float& far);
    void update();

    void framebufferDepthMap();
    void renderDepthBuffer();

};

#endif
