#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"

class Renderer {
public:
    Renderer();
    ~Renderer();
    static Renderer* getInstance();
    Camera* getCamera();
private:
    static Renderer* instance;
    Camera *m_Camera;
};

#endif