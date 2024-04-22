#ifndef SCENE_H
#define SCENE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "renderer.h"

class Scene {
public:
    Scene();
    ~Scene();
    static Scene* getInstance();
    int run();

    float m_LastX, m_LastY;
    bool m_FirstMouse;

    Camera* m_Camera;
    float m_DeltaTime;
    float m_LastTime;
    float m_PreviousFrameTime;
    float m_FramePerSecond;

private:

    static Scene* instance;
    GLFWwindow *m_Window;
    Renderer *m_Renderer;


    int setup();
    int initGlad();
    void getRenderer();
    void setGlfwCallbacks();
    void configureGlobalState();
    void updateFps();

    void processInput(GLFWwindow* window);

};

#endif
