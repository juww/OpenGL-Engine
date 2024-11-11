#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"
#include "light.h"
#include "shader_m.h"
#include "skybox.h"
#include "loadModel.h"
#include "FramebufferManager.h"

#include "cube.h"
#include "sphere.h"
#include "plane.h"
#include "water.h"
#include "waterFFT.h"
#include "computeShader.h"

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
    FramebufferManager* m_FBManager;

    std::vector<Light> m_Lights;
    loadModel* m_Model;
    Skybox* m_Skybox;
    Plane* m_Plane;
    Cube* m_LightCube;
    Sphere* m_Sphere;
    Water* m_Water;
    WaterFFT* m_WaterFFT;

    Shader* m_ModelShader;
    Shader* m_GrassShader;
    Shader* m_PlaneShader;
    Shader* m_PatchPlaneShader;
    Shader* m_LightCubeShader;
    Shader* m_SkyboxShader;
    Shader* m_WaterShader;
    Shader* m_SphereShader;

    Shader* m_NormalLineShader;

    ComputeShader* m_NoiseShader;

    Shader* m_FramebufferShader;

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