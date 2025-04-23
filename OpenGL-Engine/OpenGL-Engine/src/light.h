#pragma once
#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

class Light {
public:
    enum lightType {
        NONE,
        DIRECTION_LIGHT,
        POSITION_LIGHT,
        SPOT_LIGHT
    };

    lightType m_LightType;
    glm::vec3 m_Direction;
    glm::vec3 m_Position;
    glm::vec3 m_SpotLight;

    glm::mat4 m_LightProjection, m_LightView;

    float m_Constant;
    float m_Linear;
    float m_Quadratic;

    glm::vec3 m_Color;

    Light();
    ~Light();
    void setDirectionLight(const glm::vec3& direction);
    void setPosLight(const glm::vec3& position, const float& constant, const float& linear, const float& quadratic);
    void setSpotLight(const glm::vec3& spotlight);
    void setLightView(const glm::vec3& lookAtPosition, const glm::vec3& up);
    void setProjectionOrtho(const glm::vec4& dimension, const float& near, const float& far);
    void setProjectionPerspective(const float& fow, const float& aspect, const float& near, const float& far);

    void setColor(glm::vec3 color);
};

#endif // LIGHT_H
