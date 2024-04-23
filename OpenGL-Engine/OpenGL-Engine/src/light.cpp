#include "light.h"

Light::Light() : m_Direction(glm::vec3(0.0f, -1.0, 0.0f)), m_Position(glm::vec3(0.0f)) {
    m_Constant = 0.0f;
    m_Linear = 0.0f;
    m_Quadratic = 0.0f;
    m_Color = glm::vec3(0.0f);
}

Light::~Light() {

}

// set direction light
void Light::setLight(const glm::vec3& direction, const glm::vec3& color) {
    m_Direction = direction;
    m_Color = color;
}

// set point light or object light
void Light::setLight(const glm::vec3& position, const float& constant, const float& linear, const float& quadratic, const glm::vec3& color) {
    m_Direction = glm::vec3(0.0f);
    m_Position = position;
    m_Constant = constant;
    m_Linear = linear;
    m_Quadratic = quadratic;
    m_Color = color;
}

void Light::setColor(glm::vec3 color) {
    m_Color = color;
}