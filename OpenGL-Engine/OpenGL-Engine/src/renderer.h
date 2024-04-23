#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"
#include "light.h"
#include "shader_m.h"
#include "skybox.h"
#include "loadModel.h"

#include "cube.h"
#include "plane.h"

#include <vector>
#include <map>

class Renderer {
public:
    Renderer();
    ~Renderer();
    static Renderer* getInstance();

    Camera* getCamera();
    void start();
    void render(float currentTime);

private:

    static Renderer* instance;
    Camera* m_Camera;

    std::vector<Light> m_Lights;
    loadModel* m_Model;
    Skybox* m_Skybox;
    Plane* m_Plane;
    Cube* m_LightCube;

    Shader* m_ModelShader;
    Shader* m_GrassShader;
    Shader* m_PlaneShader;
    Shader* m_LightCubeShader;
    Shader* m_SkyboxShader;

    void configureGlobalState();
    void setupLights();
    void setupShaders();
    void initModel();

    void setModelShader(const glm::mat4& projection, const glm::mat4& view);

    // blm tentu mau dimasukin ke sini
    // global shader storage
    // 1 object - 1 shader
    std::map<std::string, Shader*> m_Shaders;
    // jadiin pointer param global
    void initGUIParam();
};

#endif