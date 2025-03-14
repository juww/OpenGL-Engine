#include "renderer.h"

#include "GUI.h"

Renderer* Renderer::instance = nullptr;

GUI::TerrainParam tp(4, 27.9f, 4, 0.5f, 2.0f, {0.0f, 0.0f}, 5.0f);
GUI::GrassParam gp(3.0f, 0.5f, 1.12f, 0.7f);
GUI::WaterParam wp(1.73f, 0.83f, 2.0f, 0.0f, 4.3f, 32);
GUI::FogDistanceParam fdp(0.03f, 150.0f, 0.1f, glm::vec3(1.0f));

Renderer::Renderer() {
    m_Camera = new Camera(glm::vec3(0.0f, 10.0f, 5.0f));
    m_Lights.clear();
    m_Shaders.clear();
    m_Skybox = new Skybox();
    m_Plane = nullptr;
    m_LightCube = nullptr;
    m_Sphere = nullptr;
    m_PBRShader = nullptr;

    m_FBManager = m_FBManager->getInstance();
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


void Renderer::configureGlobalState() {
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_FRAMEBUFFER_SRGB); // gamma correction 
}

void Renderer::setupLights() {
    Light l1;
    l1.setLight(glm::vec3(-0.2f, -1.0f, -0.3f));
    m_Lights.push_back(l1);
    l1.setLight(glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, 0.0f, 1.0f);
    m_Lights.push_back(l1);
}

void Renderer::setupShaders() {

    // build and compile our shader zprogram
    // ------------------------------------
    m_ModelShader = new Shader("cube.vs", "cube.fs", "normalMapping.gs");
    m_PlaneShader = new Shader("plane.vs", "plane.fs");
    m_GrassShader = new Shader("grass.vs", "grass.fs");
    m_LightCubeShader = new Shader("light_cube.vs", "light_cube.fs");
    m_SkyboxShader = new Shader("skybox.vs", "skybox.fs");
    m_WaterShader = new Shader("water.vs", "water.fs");
    m_SphereShader = new Shader("sphere.vs", "sphere.fs");
    m_PBRShader = new Shader("pbr.vs", "pbr.fs"/*, "normalMapping.gs"*/);

    m_PatchPlaneShader = new Shader("patchPlane.vs", "patchPlane.fs");
    m_PatchPlaneShader->setTessellationShader("TessellationControlShader.tcs", "TessellationEvaluationShader.tes");

    m_NormalLineShader = new Shader("normalLine.vs", "normalLine.fs", "normalLine.gs");
    //Shader skeletalModel("skeletal.vs", "skeletal.fs", "skeletal.gs");

    // compute shader
    m_NoiseShader = new ComputeShader("noise.sc");

    // framebuffer shader
    m_FramebufferShader = new Shader("framebufferShader.vs", "framebufferShader.fs");
}

void Renderer::initModel() {
    //const std::string& pathfile = "res/models/simpleSkin/scene.gltf";
    //const std::string& pathfile = "res/models/model_avatar/model_external.gltf";
    const std::string& pathfile = "res/models/phoenix_bird/scene.gltf";
    m_Model = new loadModel(pathfile.c_str());

    m_Model->animator.doAnimation(0);
}

void Renderer::start() {
    configureGlobalState();
    setupLights();
    setupShaders();

    initModel();

    m_Plane = new Plane(65);
    m_Plane->InitTerrainChunk(1, 64.0f, m_Camera->Position);
    m_Plane->initGrass(50);
    m_Plane->setAllUniform(m_PlaneShader);
    m_Plane->grass.generateNoiseMap(m_GrassShader, 1, 10.0f, 4, 1.5f, 2.0f, { 0.0f,0.0f });

    m_Plane->GenerateNoiseMap(m_LightCubeShader, m_NoiseShader);
    m_Plane->generatePlaneWithPatch(64, 64);

    m_LightCube = new Cube();
    m_LightCube->initialize();
    m_LightCube->pos = glm::vec3(5.0f, 10.0f, 5.0f);
    m_LightCube->scale = glm::vec3(0.2f);
    //m_LightCube->localTransform();

    m_Sphere = new Sphere(50, 2.0f);
    //m_Sphere->createHemisphere();
    m_Sphere->icosphere(5);
    m_Sphere->loadMaterials();
    //m_Sphere->cubesphere(5);

    m_Water = new Water();
    m_Water->initialize(1536, 1024, 8.0f);

    //m_WaterFFT = new WaterFFT(256);
    //m_WaterFFT->initialSpectrum();
    //m_WaterFFT->conjugateSpectrum();
    //m_WaterFFT->update(0.0f);
    
    m_Skybox = new Skybox();

    m_FBManager->setScreenSpace();
    m_FBManager->createDepthStencilFramebuffer();
    m_FBManager->shaderConfig(m_FramebufferShader);
}
//nanti dipindahin ke class model

void Renderer::setModelShader(const glm::mat4& projection, const glm::mat4& view) {
    m_ModelShader->use();
    int light_n = m_Lights.size();
    m_ModelShader->setInt("light_n", light_n);
    for (int i = 0; i < light_n; i++) {
        m_ModelShader->setInt("lightID[" + std::to_string(i) + "]", i + 1);
        m_ModelShader->setVec3("lightDirection[" + std::to_string(i) + "]", m_Lights[i].m_Direction);
        m_ModelShader->setVec3("lightPosition[" + std::to_string(i) + "]", m_Lights[i].m_Position);
        m_ModelShader->setVec3("lightColor[" + std::to_string(i) + "]", m_Lights[i].m_Color);
    }

    m_ModelShader->setVec3("viewPos", m_Camera->Position);
    m_ModelShader->setMat4("projection", projection);
    m_ModelShader->setMat4("view", view);
    m_ModelShader->setInt("hasBone", 1);

}

void Renderer::render(float currentTime, float deltaTime) {

    m_FBManager->bindFramebuffers();

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(m_Camera->Zoom), m_Camera->Aspect, 0.1f, 150.0f);
    glm::mat4 view = m_Camera->GetViewMatrix();

    setModelShader(projection, view);
    m_Model->update(m_ModelShader, deltaTime);
    m_Model->DrawModel(m_ModelShader);

    bool changeParam = GUI::proceduralTerrainParam(tp.m_Seed, tp.m_Scale, tp.m_Octaves, tp.m_Persistence, tp.m_Lacunarity, tp.m_OffsetV, tp.m_Amplitude);
    GUI::grassParam(gp.m_Frequency, gp.m_Amplitude, gp.m_Scale, gp.m_Drop);

    // draw plane
    m_Plane->update(m_Camera->Position, tp.m_Seed, tp.m_Scale, tp.m_Octaves, tp.m_Persistence, tp.m_Lacunarity, tp.m_OffsetV, tp.m_Amplitude, changeParam, m_NoiseShader);
    m_Plane->draw(m_PlaneShader, projection, view);
    m_Plane->drawGrass(m_GrassShader, projection, view, currentTime, gp.m_Frequency, gp.m_Amplitude, gp.m_Scale, gp.m_Drop);

    m_Plane->drawNoiseTexture(m_LightCubeShader, m_NoiseShader, projection, view, currentTime);
    m_Plane->drawNoiseCPU(m_LightCubeShader, projection, view, currentTime);

    m_Plane->drawPatchPlane(m_PatchPlaneShader, projection, view, 65, 65);

    m_LightCube->update(currentTime * 0.1f);
    m_LightCube->draw(m_LightCubeShader, projection, view);

    GUI::waterParam(wp.m_Amplitude, wp.m_Frequency, wp.m_Speed, wp.m_WaveCount);

    m_Water->setParameter(m_WaterShader, wp.m_Amplitude, wp.m_Frequency, currentTime, wp.m_Speed, wp.m_Seed, wp.m_SeedIter, wp.m_WaveCount, m_Camera->Position);
    //m_Water->draw(m_WaterShader, projection, view);
    //m_WaterFFT->drawTexture(m_LightCubeShader, projection, view);

    m_Skybox->draw(m_SkyboxShader, projection, glm::mat4(glm::mat3(m_Camera->GetViewMatrix())));

    m_Sphere->draw(m_PBRShader, projection, view, m_Camera->Position, currentTime * 0.1f, m_Skybox->cubemapTexture, m_LightCube->pos);
    //m_Sphere->drawNormalLine(m_NormalLineShader, projection, view);

    GUI::fogDistanceParam(fdp.m_Near, fdp.m_Far, fdp.m_Density);
    m_FBManager->setFogDistance(m_FramebufferShader, fdp.m_Near, fdp.m_Far, fdp.m_Density, fdp.m_Color);
    m_FBManager->draw(m_FramebufferShader);

}