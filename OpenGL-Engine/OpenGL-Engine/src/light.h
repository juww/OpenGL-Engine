#pragma once
#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

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

    float m_Constant;
    float m_Linear;
    float m_Quadratic;
    float m_Radius;

    glm::vec3 m_Color;

    Light();
    ~Light();
    void setDirectionLight(const glm::vec3& direction);
    void setPosLight(const glm::vec3& position, const float& constant, const float& linear, const float& quadratic);
    void setSpotLight(const glm::vec3& spotlight);

    void setColor(glm::vec3 color);
};

#endif // LIGHT_H
