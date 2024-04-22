#include "renderer.h"

Renderer* Renderer::instance = nullptr;

Renderer::Renderer() {
    m_Camera = new Camera(glm::vec3(0.0f, 0.0f, 5.0f));
}

Renderer::~Renderer() {

}

Renderer* Renderer::getInstance() {
    if (instance == nullptr) {
        instance = new Renderer();
    }
    return instance;
}

Camera* Renderer::getCamera() {
    return m_Camera;
}