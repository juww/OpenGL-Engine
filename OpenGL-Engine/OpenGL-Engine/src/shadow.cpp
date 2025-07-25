#include "shadow.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glad/glad.h>

Shadow::Shadow() {
    lightDirection = glm::vec3(0.0f, 1.0f, 0.0f);
    lightPoV = glm::vec3(0.0f, 0.0f, 0.0f);

    lightProjection = glm::mat4(1.0f);
    lightView = glm::mat4(1.0f);
    lightSpaceMatrix = glm::mat4(1.0f);
    width = 0;
    height = 0;
    aspect = 0.0f;

    shadowMappingShader = nullptr;
    depthFBO = 0;
    depthMap = 0;
}

Shadow::~Shadow() {

}

void Shadow::setShadowSizeScreen(unsigned int w, unsigned int h) {
    width = w;
    height = h;
    aspect = (float)width / (float)height;
}

void Shadow::setShader(Shader* shader) {
    shadowMappingShader = shader;
}

void Shadow::setLightPoV(glm::vec3 lightDir, float distance, glm::vec3 LookAtPosition) {
    lightPoV = (lightDir * distance) + LookAtPosition;
}

void Shadow::setLightView(glm::vec3 lookAtPosition, const glm::vec3& up) {
    lightView = glm::lookAt(lightPoV, lookAtPosition, up);
}

void Shadow::setProjectionOrtho(const glm::vec4& dimension, const float& near, const float& far) {
    lightProjection = glm::ortho(dimension.x, dimension.y, dimension.z, dimension.w, near, far);
}

void Shadow::setProjectionPerspective(const float& fow, const float& near, const float& far) {
    lightProjection = glm::perspective(fow, aspect, near, far);
}

void Shadow::update() {

}

void Shadow::framebufferDepthMap() {

    if (depthFBO == 0) {
        glGenFramebuffers(1, &depthFBO);
    }

    if (depthMap == 0) {
        glGenTextures(1, &depthMap);
    }

    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    //shadowType = 1;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Shadow::renderDepthBuffer() {

    lightSpaceMatrix = lightProjection * lightView;

    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glViewport(0, 0, width, height);
    glClear(GL_DEPTH_BUFFER_BIT);

    shadowMappingShader->use();
    shadowMappingShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    for (auto& obj : objects) {
        RenderObject& ro = obj.second;
        shadowMappingShader->setMat4("model", ro.model);

        glBindVertexArray(ro.vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ro.ebo);
        glDrawElements(GL_TRIANGLES, ro.count, ro.type, (void*)0);

        glBindVertexArray(0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
