#include "light.h"

// set direction light
void Light::setLight(const glm::vec3& direction, const glm::vec3& color) {
    m_Direction = direction;
    m_Color = color;
}

// set point light or object light
void Light::setLight(const glm::vec3& position, const float& constant, const float& linear, const float& quadratic, const glm::vec3& color) {
    m_Position = position;
    m_Constant = constant;
    m_Linear = linear;
    m_Quadratic = quadratic;
    m_Color = color;
}