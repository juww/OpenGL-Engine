#include "FramebufferManager.h"

FramebufferManager::FramebufferManager() {
    instance = nullptr;
}

FramebufferManager::~FramebufferManager() {

}

FramebufferManager* FramebufferManager::getInstance() {
    if (instance == nullptr) {
        instance = new FramebufferManager();
    }
    return instance;
}

void createFramebuffer() {
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        // execute victory dance
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//do not forget to delete
void deleteFramebuffer() {
    glDeleteFramebuffers(1, &fbo);
}