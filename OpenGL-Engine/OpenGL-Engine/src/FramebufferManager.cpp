#include "FramebufferManager.h"

FramebufferManager* FramebufferManager::instance = nullptr;

FramebufferManager::FramebufferManager() {
    m_ScreenWidth = 1280;
    m_ScreenHeight = 720;
    m_Fbo = 0;
    m_Vao = 0;
    m_ScreenVertices.clear();
}

FramebufferManager::~FramebufferManager() {

}

FramebufferManager* FramebufferManager::getInstance() {
    if (instance == nullptr) {
        instance = new FramebufferManager();
    }
    return instance;
}

void FramebufferManager::setScreenSpace() {
    m_ScreenVertices = std::vector<float>({
        // positions  // texCoords
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f
    });

    glGenVertexArrays(1, &m_Vao);
    glBindVertexArray(m_Vao);

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    const int sizeBuffer = sizeof(float) * 6 * (2 + 2);
    glBufferData(GL_ARRAY_BUFFER, sizeBuffer, &m_ScreenVertices.at(0), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void FramebufferManager::createDepthStencilFramebuffer() {

    glGenFramebuffers(1, &m_Fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);

    glGenTextures(1, &m_Texture);
    glBindTexture(GL_TEXTURE_2D, m_Texture);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_ScreenWidth, m_ScreenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_ScreenWidth, m_ScreenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_Texture, 0);

    glGenRenderbuffers(1, &m_Rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_Rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_ScreenWidth, m_ScreenHeight);
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_Rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FramebufferManager::shaderConfig(Shader* shader) {
    shader->use();
    shader->setInt("screenTexture", 0);
}


void FramebufferManager::bindFramebuffers() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);
    glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
}

void FramebufferManager::draw(Shader* shader) {
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    // clear all relevant buffers
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
    glClear(GL_COLOR_BUFFER_BIT);

    shader->use();
    glBindVertexArray(m_Vao);
    glBindTexture(GL_TEXTURE_2D, m_Texture);	// use the color attachment texture as the texture of the quad plane
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
//do not forget to delete
void FramebufferManager::deleteFramebuffer(unsigned int &fbo) {
    glDeleteFramebuffers(1, &fbo);
    fbo = 0;
}