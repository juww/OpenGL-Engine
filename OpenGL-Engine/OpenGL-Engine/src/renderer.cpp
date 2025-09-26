#include "renderer.h"

#include "GUI.h"

Renderer* Renderer::instance = nullptr;

GUI::TerrainParam tp(4, 27.9f, 4, 0.5f, 2.0f, {0.0f, 0.0f}, 5.0f);
GUI::GrassParam gp(3.0f, 0.5f, 1.12f, 0.7f);
GUI::WaterParam wp(1.73f, 0.83f, 2.0f, 0.0f, 4.3f, 32);
GUI::FogDistanceParam fdp(0.03f, 1.0f, 0.1f, glm::vec3(1.0f));
GUI::PBRParam pbr;
GUI::DeferredParam g_DeferredParam;

Renderer::Renderer() {
    m_Camera = new Camera(glm::vec3(0.0f, 10.0f, 5.0f));
    m_Lights.clear();
    m_Shaders.clear();
    m_Skybox = new Skybox();
    m_Plane = nullptr;
    m_Terrain = nullptr;
    m_Spheres.clear();
    m_PBRShader = nullptr;

    m_ScreenWidth = 0;
    m_ScreenHeight = 0;
    m_FBManager = m_FBManager->getInstance();

    m_LightCube.clear();
    m_Shadow = nullptr;
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

void Renderer::setScreenSize(int width, int height) {
    m_ScreenWidth = width;
    m_ScreenHeight = height;
}

void Renderer::configureGlobalState() {
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_FRAMEBUFFER_SRGB); // gamma correction 
    glDepthFunc(GL_LEQUAL);
    // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

// temporary global variable light
glm::vec3 g_LightDirection;
std::vector<Light> g_lightPosition;
float g_LightDist;
float g_Dimension;
std::map<unsigned int, std::string> g_GBuffer;

void Renderer::setupLights() {
    //class Light is shit,, need to change
    
    g_LightDirection = glm::normalize(glm::vec3(-2.0f, 3.0f, 1.0f));
    g_LightDist = 50.0f;
    g_Dimension = g_LightDist + 2.0f;

    const float constant = 1.0f;
    const float linear = 0.7f;
    const float quadratic = 1.8f;

    Cube c;
    c.initialize();
    c.scale = glm::vec3(0.2f);
    m_LightCube.push_back(c);

    for (unsigned int i = 0; i < 32; i++) {
        Light tlight;
        float xPos = static_cast<float>(((rand() % 100) / 100.0) * 30.0 - 15.0);
        float yPos = static_cast<float>(((rand() % 100) / 100.0) * 20.0 - 10.0);
        float zPos = static_cast<float>(((rand() % 100) / 100.0) * 15.0 - 7.5);
        tlight.setPosLight(glm::vec3(xPos, yPos, zPos), constant, linear, quadratic);
        tlight.m_Position += glm::vec3(0.0f, 10.0f, -15.0f);
        // also calculate random color
        float rColor = static_cast<float>(((rand() % 100) / 100.0f));
        float gColor = static_cast<float>(((rand() % 100) / 100.0f));
        float bColor = static_cast<float>(((rand() % 100) / 100.0f));
        glm::vec3 lightColor(rColor, gColor, bColor);
        tlight.setColor(lightColor);
        float maxBrightness = std::fmaxf(std::fmaxf(tlight.m_Color.r, tlight.m_Color.g), tlight.m_Color.b) * 2.0f;
        float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
        tlight.m_Radius = radius;
        c.pos = tlight.m_Position;
        c.color = tlight.m_Color;
        g_lightPosition.push_back(tlight);
        m_LightCube.push_back(c);
    }
}

void Renderer::setupShaders() {

    // build and compile our shader zprogram
    // ------------------------------------
    m_ModelShader = new Shader("pbr.vs", "pbr.fs"/*, "normalMapping.gs"*/);
    m_PlaneShader = new Shader("plane.vs", "plane.fs");
    m_GrassShader = new Shader("grass.vs", "grass.fs");
    m_LightCubeShader = new Shader("light_cube.vs", "light_cube.fs");
    m_SkyboxShader = new Shader("skybox.vs", "skybox.fs");
    m_WaterShader = new Shader("water.vs", "water.fs");
    m_SphereShader = new Shader("sphere.vs", "sphere.fs");
    m_PBRShader = new Shader("pbr.vs", "pbr.fs"/*, "normalMapping.gs"*/);

    m_DeferredShader = new Shader("deferredShader.vs", "deferredShader.fs");
    
    m_PatchPlaneShader = new ShaderT("patchPlane.vs", "patchPlane.fs", "", 
        "TessellationControlShader.tcs", "TessellationEvaluationShader.tes");

    m_NormalLineShader = new Shader("normalLine.vs", "normalLine.fs", "normalLine.gs");
    m_DebugShader = new Shader("debugPlane.vs", "debugPlane.fs");

    // compute shader
    m_NoiseShader = new ComputeShader("noise.sc");

    // framebuffer shader
    m_FramebufferShader = new Shader("framebufferShader.vs", "framebufferShader.fs");
    m_GBufferShader = new Shader("gBufferShader.vs", "gBufferShader.fs");
    m_SSAOShader = new Shader("ssaoShader.vs", "ssaoShader.fs");
    m_SSAOBlurShader = new Shader("ssaoBlurShader.vs", "ssaoBlurShader.fs");
    m_BloomGaussianBlurShader = new Shader("bloomGaussianBlur.vs", "bloomGaussianBlur.fs");
    m_BloomResultShader = new Shader("bloomResult.vs", "bloomResult.fs");
    m_IrradianceShader = new Shader("irradianceShader.vs", "irradianceShader.fs");
    m_PreFilterShader = new Shader("preFilterShader.vs", "preFilterShader.fs");
    m_LUTShader = new Shader("LUTShader.vs", "LUTShader.fs");
    m_ShadowMappingShader = new Shader("shadowMappingShader.vs", "shadowMappingShader.fs");
    m_ShadowCubeMappingShader = new Shader("shadowCubeMappingShader.vs", "shadowCubeMappingShader.fs", "shadowCubeMappingShader.gs");
    m_CombineTextureShader = new Shader("combineTextureShader.vs", "combineTextureShader.fs");
}

void Renderer::initModel() {

    const std::string& pathfile = "res/models/damaged-helmet/scene.gltf";
    m_Model = new gltf::Model();
    m_Model->loadModel(pathfile.c_str());
    m_Model->setShader(m_PBRShader);
    m_Model->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    m_Model->setScale(glm::vec3(1.0f));
    //m_Model->startPlayAnimation(0);

    const std::string& sponzaPathFile = "res/models/Sponza/glTF/Sponza.gltf";
    m_Sponza = new gltf::Model();
    m_Sponza->loadModel(sponzaPathFile.c_str());
    m_Sponza->setShader(m_PBRShader);
    m_Sponza->setPosition(glm::vec3(0.0f, 0.0f, -15.0f));
    m_Sponza->setScale(glm::vec3(2.0f));
    m_Sponza->getRenderObject(m_Shadow->objects);

    const std::string& dragonskinPathFile = "res/models/silver_dragonkin/scene.gltf";
    m_Dragonskin = new gltf::Model();
    m_Dragonskin->loadModel(dragonskinPathFile.c_str());
    m_Dragonskin->setShader(m_PBRShader);
    m_Dragonskin->setPosition(glm::vec3(0.0f, 15.0f, 0.0f));
    m_Dragonskin->setScale(glm::vec3(0.01f));
    m_Dragonskin->startPlayAnimation(0);
}

void Renderer::start() {
    configureGlobalState();
    setupShaders();
    setupLights();

    m_Shadow = new Shadow();

    initModel();

    //float near_plane = 1.0f, far_plane = 7.5f;
    m_Shadow->setShadowSizeScreen(1024, 1024);
    m_Shadow->setShader(m_ShadowMappingShader);
    m_Shadow->setLightPoV(g_LightDirection, g_LightDist, m_Sponza->pos);
    m_Shadow->setLightView(m_Sponza->pos, glm::vec3(0.0, 1.0, 0.0));
    m_Shadow->setProjectionOrtho(glm::vec4(-g_Dimension, g_Dimension, -g_Dimension, g_Dimension), 0.01f, g_Dimension * 2.0f);
    //m_Shadow->setProjectionPerspective(glm::radians(45.0f), 0.1f, 50.0f);
    m_Shadow->framebufferDepthMap();

    m_Plane = new Plane(65);
    m_Plane->InitTerrainChunk(1, 64.0f, m_Camera->Position);
    m_Plane->setAllUniform(m_PlaneShader);
    //m_Plane->initGrass(50);
    //m_Plane->grass.generateNoiseMap(m_GrassShader, 1, 10.0f, 4, 1.5f, 2.0f, { 0.0f,0.0f });

    m_Plane->GenerateNoiseMap(m_LightCubeShader, m_NoiseShader);
    m_Plane->generatePlaneWithPatch(64, 64);

    m_Terrain = new T_Terrain();
    m_Terrain->setPredefineHeightMap("res/textures/papua_island_height_map.png");
    //m_Terrain->createPlane(64, 64);
    m_Terrain->createPlane(m_Terrain->wHeightMap / 16.0f, m_Terrain->hHeightMap / 16.0f);
    m_Terrain->setShader(m_PatchPlaneShader);
    m_Terrain->setComputeShader(m_NoiseShader);
    m_Terrain->generateNoiseTexture(256);
    //m_LightCube->localTransform();

    for (int i = 0; i < 1; i++) {
        m_Spheres.push_back(new Sphere(50, 1.0f));
        m_Spheres[i]->icosphere(4);
        //m_Sphere->createHemisphere();
        //m_Sphere->cubesphere(5);
        m_Spheres[i]->pos = glm::vec3(15.0f, 15.0f, i * 3.0f);
        if (i == 0) m_Spheres[i]->loadMaterials("res/textures/materials/wood/");
        if (i == 1) m_Spheres[i]->loadMaterials("res/textures/materials/tiles_wall/");
        if (i == 2) m_Spheres[i]->loadMaterials("res/textures/materials/windswept-wasteland-bl/");
        if (i == 3) m_Spheres[i]->loadMaterials("res/textures/materials/mud/");
        if (i == 4) m_Spheres[i]->loadMaterials("res/textures/materials/metal_hole/");
    }

    m_Water = new Water();
    m_Water->initialize(256, 256, 8.0f);

    m_WaterFFT = new WaterFFT();
    m_WaterFFT->setPos(glm::vec3(-15.0f, 6.0f, 0.0f));
    m_WaterFFT->setPlaneSize(512);
    m_WaterFFT->setTextureSize(1024);
    m_WaterFFT->createShader("waterFFT");
    m_WaterFFT->createPlane();
    m_WaterFFT->createComputeShader();
    m_WaterFFT->initTexture();
    m_WaterFFT->initUniform();
    m_WaterFFT->initializeSpectrum();

    m_Skybox = new Skybox();

    m_FBManager->setScreenSpace();
    m_FBManager->createDepthStencilFramebuffer();
    m_FBManager->shaderConfig(m_FramebufferShader);

    m_FBManager->IrradianceMapping(m_IrradianceShader, m_Skybox->cubemapTexture, m_Skybox->width, m_Skybox->height);
    m_FBManager->PreFilterMapping(m_PreFilterShader, m_Skybox->cubemapTexture, m_Skybox->width, m_Skybox->height);
    m_FBManager->BrdfLUT(m_LUTShader, m_Skybox->width, m_Skybox->height);

    m_FBManager->generateGBuffer();
    m_FBManager->setGeometryPassShader(m_GBufferShader);
    m_FBManager->copyRenderObjects(m_Shadow->objects);

    m_FBManager->genScreenSpaceAmbientOcclusion();
    m_FBManager->setSSAOShader(m_SSAOShader, m_SSAOBlurShader);

    m_FBManager->setBloomShader(m_BloomGaussianBlurShader, m_BloomResultShader);
    m_FBManager->genBloomBuffer();

    g_GBuffer[m_FBManager->gPosition] = "gPosition";
    g_GBuffer[m_FBManager->gNormal] = "gNormal";
    g_GBuffer[m_FBManager->gAlbedo] = "gAlbedo";
    g_GBuffer[m_FBManager->gNormalMap] = "gNormalMap";
    g_GBuffer[m_FBManager->gORMMap] = "gORMMap";
    g_GBuffer[m_FBManager->gDepth] = "gDepth";
    g_GBuffer[m_FBManager->gViewPosition] = "gViewPosition";
    g_GBuffer[m_FBManager->gViewNormal] = "gViewNormal";
    g_GBuffer[m_FBManager->gTexCoords] = "gTexCoords";
    g_GBuffer[m_FBManager->ssaoBufferBlur] = "SSAO";
    g_GBuffer[m_FBManager->colorBuffer] = "colorresult";
    g_GBuffer[m_FBManager->brightnessBuffer] = "brightness";

    for (auto sphere : m_Spheres) {
        sphere->materials.metallicRoughnessOcclusionTexture = m_FBManager->combineTexture(m_CombineTextureShader, 
            sphere->materials.Map, sphere->materials.width, sphere->materials.height);
    }
    for (auto& material : m_Model->materials) {
        material->metallicRoughnessOcclusionTexture = m_FBManager->combineTexture(m_CombineTextureShader,
            material->Map, material->width, material->height);
    }
    for (auto& material : m_Sponza->materials) {
        material->metallicRoughnessOcclusionTexture = m_FBManager->combineTexture(m_CombineTextureShader,
            material->Map, material->width, material->height);
    }
    for (auto& material : m_Dragonskin->materials) {
        material->metallicRoughnessOcclusionTexture = m_FBManager->combineTexture(m_CombineTextureShader,
            material->Map, material->width, material->height);
    }
}

void Renderer::DeferredRender(std::map<std::string, unsigned int>& mappers, std::vector<glm::vec3> lightPos) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_DeferredShader->use();

    m_DeferredShader->setInt("gPosition", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_FBManager->gPosition);
    m_DeferredShader->setInt("gNormal", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_FBManager->gNormal);
    m_DeferredShader->setInt("gAlbedo", 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_FBManager->gAlbedo);
    m_DeferredShader->setInt("gNormalMap", 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_FBManager->gNormalMap);
    m_DeferredShader->setInt("gORMMap", 4);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_FBManager->gORMMap);
    m_DeferredShader->setInt("gTexCoords", 5);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_FBManager->gTexCoords);
    m_DeferredShader->setInt("ssaoBuffer", 9);
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, m_FBManager->ssaoBufferBlur);

    m_DeferredShader->setVec3("lightDirection", g_LightDirection);
    m_DeferredShader->setFloat("lightLinear", g_lightPosition[0].m_Linear);
    m_DeferredShader->setFloat("lightQuadratic", g_lightPosition[0].m_Quadratic);
    for (int i = 0; i < g_lightPosition.size(); i++) {
        m_DeferredShader->setVec3("lightPosition[" + std::to_string(i) + "]", g_lightPosition[i].m_Position);
        m_DeferredShader->setVec3("lightColor[" + std::to_string(i) + "]", g_lightPosition[i].m_Color);
        m_DeferredShader->setFloat("lightRadius[" + std::to_string(i) + "]", g_lightPosition[i].m_Radius);
    }

    m_DeferredShader->setBool("useSSAO", g_DeferredParam.useSSAO);
    m_DeferredShader->setBool("useHDR", g_DeferredParam.useHDR);
    m_DeferredShader->setBool("useORM", g_DeferredParam.useORM);
    m_DeferredShader->setFloat("exposure", g_DeferredParam.exposure);
    m_DeferredShader->setFloat("ambientStrenght", g_DeferredParam.ambientStrenght);
    
    m_DeferredShader->setVec3("viewPos", m_Camera->Position);

    m_DeferredShader->setVec4("baseColor", pbr.m_BaseColor);
    m_DeferredShader->setFloat("roughnessFactor", pbr.m_RoughnessFactor);
    m_DeferredShader->setFloat("subSurface", pbr.m_SubSurface);
    m_DeferredShader->setFloat("metallicFactor", pbr.m_MetallicFactor);

    m_DeferredShader->setFloat("_Specular", pbr.m_Specular);
    m_DeferredShader->setFloat("_SpecularTint", pbr.m_SpecularTint);
    m_DeferredShader->setFloat("_Sheen", pbr.m_Sheen);
    m_DeferredShader->setFloat("_SheenTint", pbr.m_SheenTint);
    m_DeferredShader->setFloat("_Anisotropic", pbr.m_Anisotropic);
    m_DeferredShader->setFloat("_ClearCoatGloss", pbr.m_ClearCoatGloss);
    m_DeferredShader->setFloat("_ClearCoat", pbr.m_ClearCoat);

    m_DeferredShader->setInt("irradianceMap", 6);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mappers["irradianceMap"]);

    m_DeferredShader->setInt("preFilterMap", 7);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mappers["preFilterMap"]);

    m_DeferredShader->setInt("brdfLUTTexture", 8);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, mappers["brdfLUTTexture"]);

    m_FBManager->renderQuad();
    for (int i = 0; i <= 5; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Renderer::render(float currentTime, float deltaTime) {

    //m_FBManager->bindFramebuffers();

    static std::vector<glm::vec3> lightpos;
    // for now,, will be deleted or change.
    int shadowType = 1;
    if (GUI::lightSunParam(g_LightDirection, m_Shadow->depthMap)) {
        m_Shadow->setLightPoV(g_LightDirection, g_LightDist, m_Sponza->pos);
        m_Shadow->setLightView(m_Sponza->pos, glm::vec3(0.0, 1.0, 0.0));
        m_Shadow->setProjectionOrtho(glm::vec4(-g_Dimension, g_Dimension, -g_Dimension, g_Dimension), 0.01f, g_Dimension * 2.0f);
        m_LightCube[0].pos = m_Shadow->lightPoV;
    }
    if (lightpos.empty()) {
        lightpos.push_back(g_LightDirection);
        for (Light light : m_Lights) {
            if (light.m_LightType == light.POSITION_LIGHT) {
                lightpos.push_back(light.m_Position);
            }
        }
    }

    m_Shadow->renderDepthBuffer();
    m_FBManager->mappers["shadowMapping"] = m_Shadow->depthMap;

    // view/projection transformations
    glm::mat4 projection = m_Camera->GetProjectionMatrix();
    glm::mat4 view = m_Camera->GetViewMatrix();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);

    GUI::showTextureGBuffer(g_GBuffer);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GUI::useDeferredShading(g_DeferredParam);
    if (g_DeferredParam.useDeferredRender) {

        glEnable(GL_CULL_FACE);

        m_FBManager->ssaoBias = g_DeferredParam.ssaoBias;
        m_FBManager->ssaoRadius = g_DeferredParam.ssaoRadius;

        m_FBManager->drawGBuffer(projection, view);
        m_FBManager->drawSSAO(projection, view);
        m_FBManager->SSAOBlur();

        glBindFramebuffer(GL_FRAMEBUFFER, m_FBManager->bloomFBO);
        DeferredRender(m_FBManager->mappers, lightpos);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBManager->gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBManager->bloomFBO); // write to bloom buffer
        glBlitFramebuffer(0, 0, m_ScreenWidth, m_ScreenHeight, 0, 0, m_ScreenWidth, m_ScreenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_CULL_FACE);

        for (Cube cube : m_LightCube) {
            cube.draw(m_LightCubeShader, projection, view);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        m_FBManager->twoPassGaussianBlur();
        m_FBManager->bloomResult(g_DeferredParam.bloom, g_DeferredParam.exposure);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBManager->bloomFBO); 
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer

        //this second glBlitFramebuffer make the render become slower
        glBlitFramebuffer(0, 0, m_ScreenWidth, m_ScreenHeight, 0, 0, m_ScreenWidth, m_ScreenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    GUI::modelTransform("model", m_Model->pos, m_Model->rot, m_Model->angle, m_Model->scale);
    GUI::modelTransform("sponza", m_Sponza->pos, m_Sponza->rot, m_Sponza->angle, m_Sponza->scale);
    GUI::modelTransform("dragonskin", m_Dragonskin->pos, m_Dragonskin->rot, m_Dragonskin->angle, m_Dragonskin->scale);
    
    bool changeParam = GUI::proceduralTerrainParam(tp.m_Seed, tp.m_Scale, tp.m_Octaves, tp.m_Persistence, tp.m_Lacunarity, tp.m_OffsetV, tp.m_Amplitude);

    GUI::grassParam(gp);
    GUI::PBRWindow(pbr);

    // draw plane
    m_Plane->update(m_Camera->Position, tp.m_Seed, tp.m_Scale, tp.m_Octaves, tp.m_Persistence, tp.m_Lacunarity, tp.m_OffsetV, tp.m_Amplitude, changeParam, m_NoiseShader);
    m_Plane->draw(m_PlaneShader, projection, view);
    //m_Plane->drawGrass(m_GrassShader, projection, view, currentTime, gp.m_Frequency, gp.m_Amplitude, gp.m_Scale, gp.m_Drop);

    //m_Plane->drawNoiseTexture(m_LightCubeShader, m_NoiseShader, projection, view, currentTime);
    //m_Plane->drawNoiseCPU(m_LightCubeShader, projection, view, currentTime);

    m_Plane->drawPatchPlane(m_PatchPlaneShader, projection, view, 65, 65);

    m_Terrain->computeNoiseMap();
    m_Terrain->draw(projection, view, m_Camera->Position);

    //GUI::waterParam(wp);
    //m_Water->setParameter(m_WaterShader, wp.m_Amplitude, wp.m_Frequency, currentTime, wp.m_Speed, wp.m_Seed, wp.m_SeedIter, wp.m_WaveCount, m_Camera->Position);
    //m_Water->draw(m_WaterShader, projection, view);

    GUI::waterFFTParam(m_WaterFFT->waterFFTParam);
    m_WaterFFT->update(currentTime);
    //m_WaterFFT->drawDebugPlane(m_DebugShader, projection, view);
    m_WaterFFT->draw(projection, view, m_Camera->Position, m_Skybox->cubemapTexture);

    GUI::modelAnimation("model", m_Model->animator, m_Model->playAnimation);
    m_Model->setUniforms(m_Camera, currentTime, m_FBManager->mappers, lightpos, pbr, m_Shadow->lightSpaceMatrix);
    m_Model->update(deltaTime);
    m_Model->draw();

    m_Sponza->setUniforms(m_Camera, currentTime, m_FBManager->mappers, lightpos, pbr, m_Shadow->lightSpaceMatrix);
    m_Sponza->shader->setInt("useShadowMapping", 1);
    m_Sponza->update(deltaTime);
    //m_Sponza->draw();

    GUI::modelAnimation("dragonskin", m_Dragonskin->animator, m_Dragonskin->playAnimation);
    m_Dragonskin->setUniforms(m_Camera, currentTime, m_FBManager->mappers, lightpos, pbr, m_Shadow->lightSpaceMatrix);
    m_Dragonskin->update(deltaTime);
    m_Dragonskin->draw();

    for (auto sphere : m_Spheres) {
        sphere->draw(m_PBRShader, projection, view, m_Camera->Position,
            currentTime * 0.1f, m_FBManager->mappers, lightpos, pbr);
    }
    m_Skybox->draw(m_SkyboxShader, projection, glm::mat4(glm::mat3(m_Camera->GetViewMatrix())));
    //m_Sphere->drawNormalLine(m_NormalLineShader, projection, view);

    //GUI::fogDistanceParam(fdp);

    //m_FBManager->bindFramebuffers();
    //m_FBManager->setFogDistance(m_FramebufferShader, fdp.m_Near, fdp.m_Far, fdp.m_Density, fdp.m_Color);
    //m_FBManager->draw(m_FramebufferShader);
}