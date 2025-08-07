#include "scene.h"

#include "GUI.h"

#include <iostream>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tinyGLTF/tiny_gltf.h"
Scene* Scene::instance = nullptr;

bool cursorcb = true;
bool updatecb = false;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

Scene::Scene() {
    m_Window = nullptr;
    m_Renderer = nullptr;
    m_Camera = nullptr;

    m_SceneWidth = 1280;
    m_SceneHeight = 720;

    m_LastX = m_SceneWidth / 2.0f;
    m_LastY = m_SceneHeight / 2.0f;
    m_FirstMouse = true;

    m_CurrentTime = 0.0f;
    m_DeltaTime = 0.0f;
    m_LastTime = 0.0f;
    m_FramePerSecond = 0.0f;
    m_PreviousFrameTime = 0.0f;
}

Scene::~Scene() {

}

Scene *Scene::getInstance() {
    if (instance == nullptr) {
        instance = new Scene();
    }
    return instance;
}

int Scene::setup() {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(m_SceneWidth, m_SceneHeight, "LearnOpenGL", NULL, NULL);
    if (m_Window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(m_Window);

    return 0;
}

int Scene::initGlad() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    return 0;
}

void Scene::setGlfwCallbacks() {
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
    glfwSetCursorPosCallback(m_Window, mouse_callback);
    glfwSetScrollCallback(m_Window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}


void Scene::getRenderer() {
    m_Renderer = Renderer::getInstance();
    m_Camera = m_Renderer->getCamera();
}


void Scene::updateFps() {
    m_CurrentTime = static_cast<float>(glfwGetTime());
    m_FramePerSecond += 1.0f;
    m_DeltaTime = m_CurrentTime - m_LastTime;

    if (m_CurrentTime - m_PreviousFrameTime >= 1.0) {
        printf("frame per second : %f\n", m_FramePerSecond);
        printf("%f ms\n", 1000.0f / m_FramePerSecond);
        printf("deltaTime = %f \n", m_DeltaTime);
        m_FramePerSecond = 0.0f;
        m_PreviousFrameTime = m_CurrentTime;
        updatecb = true;
    }
    m_LastTime = m_CurrentTime;
}

void Scene::configViewPort() {
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(m_Window, &scrWidth, &scrHeight);

    printf("config ViewPort: %d %d\n", scrWidth, scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);
}

int Scene::run() {
    setup();
    initGlad();
    setGlfwCallbacks();
    getRenderer();
    m_Renderer->start();

    GUI::initialize(m_Window);
    m_PreviousFrameTime = static_cast<float>(glfwGetTime());
    configViewPort();
    while (!glfwWindowShouldClose(m_Window)) {
        updateFps();

        processInput(m_Window);

        GUI::GUIFrame();
        m_Camera->setAspect(m_SceneWidth, m_SceneHeight);
        m_Camera->setNearAndFarProjection(0.1f, 512.0f);

        m_Renderer->render(m_CurrentTime, m_DeltaTime);

        GUI::renderUI();

        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }
    GUI::shutDown();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Scene::processInput(GLFWwindow* window) {

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_Camera->ProcessKeyboard(FORWARD, m_DeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_Camera->ProcessKeyboard(BACKWARD, m_DeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_Camera->ProcessKeyboard(LEFT, m_DeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_Camera->ProcessKeyboard(RIGHT, m_DeltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        m_Camera->ProcessKeyboard(UP, m_DeltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        m_Camera->ProcessKeyboard(DOWN, m_DeltaTime);
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!updatecb) return;
        updatecb = false;
        if (!cursorcb) {
            cursorcb = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else {
            cursorcb = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    
    Scene* scene = Scene::getInstance();
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (scene->m_FirstMouse) {
        scene->m_LastX = xpos;
        scene->m_LastY = ypos;
        scene->m_FirstMouse = false;
    }

    float xoffset = xpos - scene->m_LastX;
    float yoffset = scene->m_LastY - ypos; // reversed since y-coordinates go from bottom to top

    scene->m_LastX = xpos;
    scene->m_LastY = ypos;
    if (cursorcb) scene->m_Camera->ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    Scene* scene = Scene::getInstance();
    scene->m_Camera->ProcessMouseScroll(static_cast<float>(yoffset));
}