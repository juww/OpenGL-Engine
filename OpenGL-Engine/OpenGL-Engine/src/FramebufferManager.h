#pragma once
#ifndef FRAMEBUFFER_MANAGER_H
#define FRAMEBUFFER_MANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class FramebufferManager {
public:
    FramebufferManager();
    ~FramebufferManager();
    static FramebufferManager* getInstance();

private:
    static FramebufferManager* instance;
};

#endif // !FRAMEBUFFER_MANAGER_H
