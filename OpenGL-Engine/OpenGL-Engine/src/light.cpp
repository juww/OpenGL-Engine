#include "light.h"

Light::Light() : m_Direction(glm::vec3(0.0f, -1.0, 0.0f)), m_Position(glm::vec3(0.0f)), m_SpotLight(glm::vec3(0.0f)) {
    m_LightType = NONE;
    m_Constant = 0.0f;
    m_Linear = 0.0f;
    m_Quadratic = 0.0f;
    m_Color = glm::vec3(0.0f);
}

Light::~Light() {

}

// set direction light
void Light::setDirectionLight(const glm::vec3& direction) {
    m_LightType = DIRECTION_LIGHT;
    m_Direction = direction;
}

// set point light or object light
void Light::setPosLight(const glm::vec3& position, const float& constant, const float& linear, const float& quadratic) {
    m_LightType = POSITION_LIGHT;
    m_Position = position;
    m_Constant = constant;
    m_Linear = linear;
    m_Quadratic = quadratic;
}

void Light::setSpotLight(const glm::vec3& spotlight) {
    m_LightType = SPOT_LIGHT;
    m_SpotLight = spotlight;
}

void Light::setColor(glm::vec3 color) {
    m_Color = color;
}

void Light::setLightView(const glm::vec3& lookAtPosition, const glm::vec3& up) {
    
    switch (m_LightType) {
        case (POSITION_LIGHT):
            m_LightView = glm::lookAt(m_Position, lookAtPosition, up);
            break;

        case (DIRECTION_LIGHT):
            m_LightView = glm::lookAt(m_Direction, lookAtPosition, up);
            break;

        default:
            m_LightView = glm::lookAt(m_Direction, lookAtPosition, up);
            break;
    }
}

void Light::setProjectionOrtho(const glm::vec4& dimension, const float& near, const float& far) {
    m_LightProjection = glm::ortho(dimension.x, dimension.y, dimension.z, dimension.w, near, far);
}

void Light::setProjectionPerspective(const float& fow, const float &aspect, const float& near, const float& far) {
    m_LightProjection = glm::perspective(fow, aspect, near, far);
}