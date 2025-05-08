#include "waterFFT.h"

#define PI 3.14159265358979323846

WaterFFT::WaterFFT() {
    pos = glm::vec3(0.0f);
    planeSize = 0;
    textureSize = 0;
    initialSpectrumTexture = 0;
    spectrumTexture = 0;

    waterShader = nullptr;
    compute_InitialSpectrum = nullptr;
    compute_UpdateSpectrum = nullptr;

    updateSpectrum = true;
}

WaterFFT::~WaterFFT() {

}

void WaterFFT::createPlane() {
    
    std::vector<glm::vec3> patchPos;
    std::vector<unsigned int> patchIndx;

    for (int i = 0; i < planeSize; i++) {
        for (int j = 0; j < planeSize; j++) {
            patchPos.push_back(glm::vec3(j, 0.0f, i));
        }
    }

    for (int i = 0; i < planeSize - 1; i++) {
        for (int j = 0; j < planeSize - 1; j++) {
            patchIndx.push_back(i * planeSize + j);
            patchIndx.push_back(i * planeSize + (j + 1));
            patchIndx.push_back((i + 1) * planeSize + (j + 1));
            patchIndx.push_back((i + 1) * planeSize + j);
        }
    }

    glGenVertexArrays(1, &patchAttribute.vao);
    glBindVertexArray(patchAttribute.vao);

    glPatchParameteri(GL_PATCH_VERTICES, 4);

    patchAttribute.indicesSize = patchIndx.size();
    glGenBuffers(1, &patchAttribute.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, patchAttribute.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, patchIndx.size() * sizeof(unsigned int), &patchIndx[0], GL_STATIC_DRAW);

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, patchPos.size() * sizeof(glm::vec3), &patchPos[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void WaterFFT::draw(glm::mat4 projection, glm::mat4 view) {

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glm::mat4 m(1.0f);
    m = glm::translate(m, pos);

    waterShader->use();

    waterShader->setFloat("width", textureSize);
    waterShader->setFloat("heigth", textureSize);

    waterShader->setMat4("projection", projection);
    waterShader->setMat4("view", view);
    waterShader->setMat4("model", m);

    attributeArray& attr = patchAttribute;
    glBindVertexArray(attr.vao);

    //shader->setInt("Textures", 0);
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, qp.tex);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, attr.ebo);
    glDrawElements(GL_PATCHES, attr.indicesSize, GL_UNSIGNED_INT, (void*)0);

    glBindVertexArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

float asuTime = 0.0f;
void WaterFFT::initializeSpectrum() {

    if (!updateSpectrum) {
        return;
    }
    //updateSpectrum = false;

    int cnt = 0;
    if (initialSpectrumTexture == 0) {
        initialSpectrumTexture = createRenderTexture(cnt++);
    }

    if (spectrumTexture == 0) {
        spectrumTexture = createRenderTexture(cnt++);
    }

    setUniform(compute_InitialSpectrum);

    compute_InitialSpectrum->use();
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, initialSpectrumTexture);
    //glBindImageTexture(0, initialSpectrumTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGB32F);

    glDispatchCompute((textureSize / 8), (textureSize / 8), 1);
    // make sure writing to image has finished before read
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    compute_PackSpectrumConjugate->use();
    compute_PackSpectrumConjugate->setInt("_N", textureSize);

    glDispatchCompute((textureSize / 8), (textureSize / 8), 1);
    // make sure writing to image has finished before read
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void WaterFFT::updateSpectrumToFFT(float frameTime) {

    setUniform(compute_UpdateSpectrum);
    compute_UpdateSpectrum->use();
    compute_UpdateSpectrum->setFloat("_FrameTime", frameTime);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, initialSpectrumTexture);
    glBindImageTexture(0, initialSpectrumTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGB32F);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, spectrumTexture);
    glBindImageTexture(1, spectrumTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGB32F);

    glDispatchCompute((textureSize / 8), (textureSize / 8), 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

unsigned int WaterFFT::createRenderTexture(int binding) {

    unsigned int tex;

    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0 + binding);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureSize, textureSize, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(binding, tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    return tex;
}

void WaterFFT::initUniform() {
    waterUniform.seed = 1234;
    waterUniform.lowCutoff = 0.0001f;
    waterUniform.highCutoff = 9000.0f;
    waterUniform.gravity = 9.81f;
    waterUniform.depth = 20.0f;
    waterUniform.repeatTime = 200.0f;
    waterUniform.speed = 1.0f;
    waterUniform.lambda = glm::vec2(1.0f, 1.0f);
    waterUniform.displacementDepthFalloff = 1.0f;
    waterUniform.normalStrength = 1.0f;
    waterUniform.normalDepthFalloff = 1.0f;

    displaySpectrum.scale = 0.2f;
    displaySpectrum.windSpeed = 15.0f;
    displaySpectrum.windDirection = 22.0f;
    displaySpectrum.fetch = 100000.0f;
    displaySpectrum.spreadBlend = 0.642f;
    displaySpectrum.swell = 1.0f;
    displaySpectrum.peakEnhancement = 3.3f;
    displaySpectrum.shortWavesFade = 0.01f;

    spectrumParam.scale = displaySpectrum.scale;
    spectrumParam.angle = displaySpectrum.windDirection / 180.0f * PI;
    spectrumParam.spreadBlend = displaySpectrum.spreadBlend;
    spectrumParam.swell = displaySpectrum.swell;
    spectrumParam.alpha = JonswapAlpha(displaySpectrum.fetch, displaySpectrum.windSpeed);
    spectrumParam.peakOmega = JonswapPeakFrequency(displaySpectrum.fetch, displaySpectrum.windSpeed);
    spectrumParam.gamma = displaySpectrum.peakEnhancement;
    spectrumParam.shortWavesFade = displaySpectrum.shortWavesFade;
}

void WaterFFT::setUniform(ComputeShader* computeShader) {
    computeShader->use();
    computeShader->setInt("_N", textureSize);
    computeShader->setInt("_Seed", waterUniform.seed);
    computeShader->setFloat("_LowCutoff", waterUniform.lowCutoff);
    computeShader->setFloat("_HighCutoff", waterUniform.highCutoff);
    computeShader->setFloat("_Gravity", waterUniform.gravity);
    computeShader->setFloat("_Depth", waterUniform.depth);

    computeShader->setFloat("_RepeatTime", waterUniform.repeatTime);
    computeShader->setFloat("_Speed", waterUniform.speed);
    computeShader->setVec2("_Lambda", waterUniform.lambda);
    computeShader->setFloat("_DisplacementDepthFalloff", waterUniform.displacementDepthFalloff);

    computeShader->setInt("_LengthScale0", 1024);
    computeShader->setInt("_LengthScale1", 256);
    computeShader->setInt("_LengthScale2", 256);
    computeShader->setInt("_LengthScale3", 256);

    computeShader->setFloat("_Spectrums[0].scale", spectrumParam.scale);
    computeShader->setFloat("_Spectrums[0].angle", spectrumParam.angle);
    computeShader->setFloat("_Spectrums[0].spreadBlend", spectrumParam.spreadBlend);
    computeShader->setFloat("_Spectrums[0].swell", spectrumParam.swell);
    computeShader->setFloat("_Spectrums[0].alpha", spectrumParam.alpha);
    computeShader->setFloat("_Spectrums[0].peakOmega", spectrumParam.peakOmega);
    computeShader->setFloat("_Spectrums[0].gamma", spectrumParam.gamma);
    computeShader->setFloat("_Spectrums[0].shortWavesFade", spectrumParam.shortWavesFade);
}

void WaterFFT::createDebugPlane() {

    float vertex[20] = {
    -0.5f, 0.0f, -0.5f, 0.0f, 0.0f,
     0.5f, 0.0f, -0.5f, 1.0f, 0.0f,
     0.5f, 0.0f,  0.5f, 1.0f, 1.0f,
    -0.5f, 0.0f,  0.5f, 0.0f, 1.0f,
    };
    unsigned int indices[6] = {
        0,2,3,  1,2,0
    };
    for (int i = 0; i < 2; i++) {

        glGenVertexArrays(1, &debug[i].vao);
        glBindVertexArray(debug[i].vao);

        glGenBuffers(1, &debug[i].ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, debug[i].ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), &indices, GL_STATIC_DRAW);

        unsigned int vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 5, &vertex, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }
}

void WaterFFT::drawDebugPlane(Shader *shader, glm::mat4 projection, glm::mat4 view) {


    for (int i = 0; i < 2; i++) {
        glBindVertexArray(debug[i].vao);
        glm::mat4 m(1.0f);
        float xx = i * 2.0f;
        m = glm::translate(m, glm::vec3(-1.0f + xx, 7.0f, 0.0f));

        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        shader->setMat4("model", m);

        if (i == 0) {
            shader->setInt("Textures", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, initialSpectrumTexture);
        }
        if (i == 1) {
            shader->setInt("Textures", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, spectrumTexture);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, debug[i].ebo);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

        glBindVertexArray(0);
    }
}

void WaterFFT::createShader(std::string filename) {
    std::string vs = filename + ".vs";
    std::string fs = filename + ".fs";
    std::string tcs = filename + ".tcs";
    std::string tes = filename + ".tes";

    waterShader = new ShaderT(vs, fs, "", tcs, tes);
}

void WaterFFT::createComputeShader() {
    std::string computePath = "waterFFT_InitSpectrum.sc";
    compute_InitialSpectrum = new ComputeShader(computePath);

    computePath = "waterFFT_UpdateSpectrum.sc";
    compute_UpdateSpectrum = new ComputeShader(computePath);

    computePath = "waterFFT_PackSpectrumConjugate.sc";
    compute_PackSpectrumConjugate = new ComputeShader(computePath);
}

float WaterFFT::JonswapAlpha(float fetch, float windSpeed) {
    return 0.076f * std::pow(waterUniform.gravity * fetch / windSpeed / windSpeed, -0.22f);
}

float WaterFFT::JonswapPeakFrequency(float fetch, float windSpeed) {
    return 22.0f * std::pow(windSpeed * fetch / waterUniform.gravity / waterUniform.gravity, -0.33f);
}

void WaterFFT::setPlaneSize(int n) {
    planeSize = n;
}

void WaterFFT::setTextureSize(int n) {
    textureSize = n;
}

void WaterFFT::setPos(glm::vec3 p) {
    pos = p;
}