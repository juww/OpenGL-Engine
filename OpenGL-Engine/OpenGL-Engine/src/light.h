#pragma once
#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

class Light {
public:

    glm::vec3 m_Direction;
    glm::vec3 m_Position;

    float m_Constant;
    float m_Linear;
    float m_Quadratic;

    glm::vec3 m_Color;

    Light();
    ~Light();
    void setLight(const glm::vec3& direction, const glm::vec3& color = glm::vec3(1.0f));
    void setLight(const glm::vec3& position, const float& constant, const float& linear, const float& quadratic, const glm::vec3& color = glm::vec3(1.0f));
    void setColor(glm::vec3 color);
};

#endif // LIGHT_H
