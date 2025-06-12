#include "waterFFT.h"

#define PI 3.14159265358979323846

WaterFFT::WaterFFT() {
    pos = glm::vec3(0.0f);
    planeSize = 0;
    textureSize = 0;
    initialSpectrumTexture = 0;
    spectrumTexture = 0;
    displacementTexture = 0;
    slopeTexture = 0;

    waterFFTShader = nullptr;
    compute_InitialSpectrum = nullptr;
    compute_UpdateSpectrum = nullptr;
    compute_FFTHorizontal = nullptr;
    compute_FFTVertical = nullptr;
    compute_AssembleMaps = nullptr;

    hasCreateDebugPlane = false;
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

void WaterFFT::update(float frameTime) {

    if (updateSpectrum) {
        initializeSpectrum();
    }
    updateSpectrumToFFT(frameTime);
}

void WaterFFT::draw(glm::mat4 projection, glm::mat4 view, glm::vec3 viewPos, unsigned int environmentMap) {

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glm::mat4 m(1.0f);
    m = glm::translate(m, pos);

    waterFFTShader->use();

    waterFFTShader->setMat4("projection", projection);
    waterFFTShader->setMat4("view", view);
    waterFFTShader->setMat4("model", m);

    attributeArray& attr = patchAttribute;
    glBindVertexArray(attr.vao);

    waterFFTShader->setInt("displacementTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, displacementTexture);

    waterFFTShader->setInt("slopeTexture", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, slopeTexture);

    waterFFTShader->setInt("_EnvironmentMap", 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);

    // pbr parameter uniform
    glm::vec4 waterColor = glm::vec4(6.0 / 255.0f, 66.0f / 255.0f, 115.0f / 255.0f, 1.0f);
    glm::vec3 lightDir = glm::normalize(GUI::vecColor3(waterFFTParam.waterUniform.lightDirection));
    waterFFTShader->setVec3("lightDirection", lightDir);
    waterFFTShader->setVec3("viewPos", viewPos);
    waterFFTShader->setInt("arrayTextureSize", waterFFTParam.waterUniform.arrayTextureSize);
    waterFFTShader->setVec4("baseColor", waterColor);
    for (int i = 0;i < 3; i++) {
        waterFFTParam.waterUniform.lightDirection[i] = lightDir[i];
    }

    GUI::WaterFFTParam::PBRWaterParam& pbr = waterFFTParam.PBRWater;
    waterFFTShader->setFloat("_Roughness", pbr.roughness);
    waterFFTShader->setFloat("_Metallic", pbr.metallic);
    waterFFTShader->setVec3("_SunIrradiance", GUI::vecColor3(pbr.sunIrradiance));
    waterFFTShader->setVec3("_ScatterColor", GUI::vecColor3(pbr.scatterColor));
    waterFFTShader->setVec3("_BubbleColor", GUI::vecColor3(pbr.bubbleColor));
    waterFFTShader->setVec3("_FoamColor", GUI::vecColor3(waterFFTParam.foamParam.foamColor));
    waterFFTShader->setFloat("_HeightModifier", pbr.heightModifier);
    waterFFTShader->setFloat("_FoamRoughnessModifier", pbr.foamRoughnessModifier);
    waterFFTShader->setFloat("_BubbleDensity", pbr.bubbleDensity);
    waterFFTShader->setFloat("_WavePeakScatterStrength", pbr.wavePeakScatterStrength);
    waterFFTShader->setFloat("_ScatterStrength", pbr.scatterStrength);
    waterFFTShader->setFloat("_ScatterShadowStrength", pbr.scatterShadowStrength);
    waterFFTShader->setFloat("_EnvironmentLightStrength", pbr.environmentLightStrength);

    for (int i = 0; i < TEXTURE_ARRAY_SIZE; i++) {
        waterFFTShader->setBool("useSpectrum[" + std::to_string(i) + "]", spectrumParam[i].useSpectrum);
        waterFFTShader->setFloat("tile[" + std::to_string(i) + "]", spectrumParam[i].tile);
        waterFFTShader->setFloat("foamSubtract[" + std::to_string(i) + "]", waterFFTParam.foamParam.foamSubtract[i]);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, attr.ebo);
    glDrawElements(GL_PATCHES, attr.indicesSize, GL_UNSIGNED_INT, (void*)0);

    glBindVertexArray(0);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void WaterFFT::updateSpectrumParam() {
    for (int i = 0; i < TEXTURE_ARRAY_SIZE; i++) {
        GUI::WaterFFTParam::SpectrumParam& sp = waterFFTParam.spectrumParam[i];
        spectrumParam[i].scale = sp.scale;
        spectrumParam[i].angle = sp.windDirection / 180.0f * PI;
        spectrumParam[i].spreadBlend = sp.spreadBlend;
        spectrumParam[i].swell = sp.swell;
        spectrumParam[i].alpha = JonswapAlpha(sp.fetch, sp.windSpeed);
        spectrumParam[i].peakOmega = JonswapPeakFrequency(sp.fetch, sp.windSpeed);
        spectrumParam[i].gamma = sp.peakEnhancement;
        spectrumParam[i].shortWavesFade = sp.shortWavesFade;
        spectrumParam[i].useSpectrum = sp.useSpectrum;
        spectrumParam[i].tile = sp.tile;
    }
}

void WaterFFT::initTexture() {
    if (initialSpectrumTexture == 0) {
        initialSpectrumTexture = createRenderTexture(0, TEXTURE_ARRAY_SIZE);
        debug[0].tex = initialSpectrumTexture;
    }
    if (spectrumTexture == 0) {
        spectrumTexture = createRenderTexture(1, TEXTURE_ARRAY_SIZE);
        debug[1].tex = spectrumTexture;
    }
    if (derivativeTexture == 0) {
        derivativeTexture = createRenderTexture(2, TEXTURE_ARRAY_SIZE);
        debug[2].tex = derivativeTexture;
    }
    if (displacementTexture == 0) {
        displacementTexture = createRenderTexture(3, TEXTURE_ARRAY_SIZE);
        debug[3].tex = displacementTexture;
    }
    if (slopeTexture == 0) {
        slopeTexture = createRenderTexture(4, TEXTURE_ARRAY_SIZE);
        debug[4].tex = slopeTexture;
    }
}

void WaterFFT::initializeSpectrum() {

    compute_InitialSpectrum->use();
    GUI::WaterFFTParam::WaterUniform& waterUniform = waterFFTParam.waterUniform;
    compute_InitialSpectrum->setInt("_N", textureSize);
    compute_InitialSpectrum->setInt("_ArrayTextureSize", waterFFTParam.waterUniform.arrayTextureSize);
    compute_InitialSpectrum->setInt("_Seed", waterUniform.seed);
    compute_InitialSpectrum->setFloat("_LowCutoff", waterUniform.lowCutoff);
    compute_InitialSpectrum->setFloat("_HighCutoff", waterUniform.highCutoff);
    compute_InitialSpectrum->setFloat("_Gravity", waterUniform.gravity);
    compute_InitialSpectrum->setFloat("_Depth", waterUniform.depth);
    compute_InitialSpectrum->setInt("_LengthScale0", waterFFTParam.spectrumParam[0].lengthScale);
    compute_InitialSpectrum->setInt("_LengthScale1", waterFFTParam.spectrumParam[1].lengthScale);
    compute_InitialSpectrum->setInt("_LengthScale2", waterFFTParam.spectrumParam[2].lengthScale);
    compute_InitialSpectrum->setInt("_LengthScale3", waterFFTParam.spectrumParam[3].lengthScale);

    updateSpectrumParam();
    for (int i = 0; i < TEXTURE_ARRAY_SIZE; i++) {
        compute_InitialSpectrum->setFloat("_Spectrums[" + std::to_string(i) + "].scale", spectrumParam[i].scale);
        compute_InitialSpectrum->setFloat("_Spectrums[" + std::to_string(i) + "].angle", spectrumParam[i].angle);
        compute_InitialSpectrum->setFloat("_Spectrums[" + std::to_string(i) + "].spreadBlend", spectrumParam[i].spreadBlend);
        compute_InitialSpectrum->setFloat("_Spectrums[" + std::to_string(i) + "].swell", spectrumParam[i].swell);
        compute_InitialSpectrum->setFloat("_Spectrums[" + std::to_string(i) + "].alpha", spectrumParam[i].alpha);
        compute_InitialSpectrum->setFloat("_Spectrums[" + std::to_string(i) + "].peakOmega", spectrumParam[i].peakOmega);
        compute_InitialSpectrum->setFloat("_Spectrums[" + std::to_string(i) + "].gamma", spectrumParam[i].gamma);
        compute_InitialSpectrum->setFloat("_Spectrums[" + std::to_string(i) + "].shortWavesFade", spectrumParam[i].shortWavesFade);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, initialSpectrumTexture);
    glBindImageTexture(0, initialSpectrumTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

    glDispatchCompute((textureSize / 8), (textureSize / 8), 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    compute_PackSpectrumConjugate->use();
    compute_PackSpectrumConjugate->setInt("_N", textureSize);
    compute_PackSpectrumConjugate->setInt("_ArrayTextureSize", waterFFTParam.waterUniform.arrayTextureSize);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, initialSpectrumTexture);
    glBindImageTexture(0, initialSpectrumTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

    glDispatchCompute((textureSize / 8), (textureSize / 8), 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void WaterFFT::inverseFFT() {

    compute_FFTHorizontal->use();
    compute_FFTHorizontal->setInt("_ArrayTextureSize", waterFFTParam.waterUniform.arrayTextureSize);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, spectrumTexture);
    glBindImageTexture(1, spectrumTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D_ARRAY, derivativeTexture);
    glBindImageTexture(2, derivativeTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

    glDispatchCompute(1, textureSize, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    compute_FFTVertical->use();
    compute_FFTVertical->setInt("_ArrayTextureSize", waterFFTParam.waterUniform.arrayTextureSize);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, spectrumTexture);
    glBindImageTexture(1, spectrumTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D_ARRAY, derivativeTexture);
    glBindImageTexture(2, derivativeTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

    glDispatchCompute(1, textureSize, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void WaterFFT::updateSpectrumToFFT(float frameTime) {

    compute_UpdateSpectrum->use();
    compute_UpdateSpectrum->setInt("_N", textureSize);
    compute_UpdateSpectrum->setInt("_ArrayTextureSize", waterFFTParam.waterUniform.arrayTextureSize);
    compute_UpdateSpectrum->setFloat("_Gravity", waterFFTParam.waterUniform.gravity);
    compute_UpdateSpectrum->setFloat("_RepeatTime", waterFFTParam.waterUniform.repeatTime);
    compute_UpdateSpectrum->setFloat("_FrameTime", frameTime);
    compute_UpdateSpectrum->setInt("_LengthScale0", waterFFTParam.spectrumParam[0].lengthScale);
    compute_UpdateSpectrum->setInt("_LengthScale1", waterFFTParam.spectrumParam[1].lengthScale);
    compute_UpdateSpectrum->setInt("_LengthScale2", waterFFTParam.spectrumParam[2].lengthScale);
    compute_UpdateSpectrum->setInt("_LengthScale3", waterFFTParam.spectrumParam[3].lengthScale);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, initialSpectrumTexture);
    glBindImageTexture(0, initialSpectrumTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, spectrumTexture);
    glBindImageTexture(1, spectrumTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D_ARRAY, derivativeTexture);
    glBindImageTexture(2, derivativeTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

    glDispatchCompute((textureSize / 8), (textureSize / 8), 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    inverseFFT();

    compute_AssembleMaps->use();
    glm::vec2 lambda(waterFFTParam.waterUniform.lambda[0], waterFFTParam.waterUniform.lambda[1]);
    compute_AssembleMaps->setVec2("_Lambda", lambda);
    compute_AssembleMaps->setInt("_ArrayTextureSize", waterFFTParam.waterUniform.arrayTextureSize);
    compute_AssembleMaps->setFloat("_FoamBias", waterFFTParam.foamParam.foamBias);
    compute_AssembleMaps->setFloat("_FoamDecayRate", waterFFTParam.foamParam.foamDecayRate);
    compute_AssembleMaps->setFloat("_FoamAdd", waterFFTParam.foamParam.foamAdd);
    compute_AssembleMaps->setFloat("_FoamThreshold", waterFFTParam.foamParam.foamThreshold);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, spectrumTexture);
    glBindImageTexture(1, spectrumTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D_ARRAY, derivativeTexture);
    glBindImageTexture(2, derivativeTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D_ARRAY, displacementTexture);
    glBindImageTexture(3, displacementTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D_ARRAY, slopeTexture);
    glBindImageTexture(4, slopeTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

    glDispatchCompute((textureSize / 8), (textureSize / 8), 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

unsigned int WaterFFT::createRenderTexture(int binding, int arrayTextureSize) {

    unsigned int tex;

    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0 + binding);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA32F, textureSize, textureSize, arrayTextureSize);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA32F, textureSize, textureSize, arrayTextureSize, 0, GL_RGBA, GL_FLOAT, nullptr);
    for (int i = 0; i < arrayTextureSize; i++) {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, textureSize, textureSize, 1, GL_RGBA32F, GL_FLOAT, nullptr);
    }

    return tex;
}

void WaterFFT::initUniform() {
    GUI::WaterFFTParam::WaterUniform& waterUniform = waterFFTParam.waterUniform;
    waterUniform.arrayTextureSize = 1;
    waterUniform.seed = 1234;
    waterUniform.lightDirection[0] = -1.0f; waterUniform.lightDirection[1] = -1.0f; waterUniform.lightDirection[2] = -1.0f;
    waterUniform.lowCutoff = 0.0001f;
    waterUniform.highCutoff = 9000.0f;
    waterUniform.gravity = 9.81f;
    waterUniform.depth = 20.0f;
    waterUniform.repeatTime = 200.0f;
    waterUniform.speed = 1.0f;
    waterUniform.lambda[0] = 1.0f; waterUniform.lambda[1] = 1.0f;
    waterUniform.displacementDepthFalloff = 1.0f;
    waterUniform.normalStrength = 1.0f;
    waterUniform.normalDepthFalloff = 1.0f;

    waterFFTParam.spectrumParam[0].lengthScale = 256.0f;
    waterFFTParam.spectrumParam[0].tile = 128.0f;
    waterFFTParam.spectrumParam[0].scale = 0.2f;
    waterFFTParam.spectrumParam[0].windSpeed = 15.0f;
    waterFFTParam.spectrumParam[0].windDirection = 22.0f;
    waterFFTParam.spectrumParam[0].fetch = 100000.0f;
    waterFFTParam.spectrumParam[0].spreadBlend = 0.642f;
    waterFFTParam.spectrumParam[0].swell = 1.0f;
    waterFFTParam.spectrumParam[0].peakEnhancement = 3.3f;
    waterFFTParam.spectrumParam[0].shortWavesFade = 0.01f;
    waterFFTParam.spectrumParam[0].useSpectrum = true;

    waterFFTParam.spectrumParam[1].lengthScale = 128.0f;
    waterFFTParam.spectrumParam[1].tile = 64.0f;
    waterFFTParam.spectrumParam[1].scale = 0.01f;
    waterFFTParam.spectrumParam[1].windSpeed = 2.0f;
    waterFFTParam.spectrumParam[1].windDirection = 59.0f;
    waterFFTParam.spectrumParam[1].fetch = 1000.0f;
    waterFFTParam.spectrumParam[1].spreadBlend = 0.0f;
    waterFFTParam.spectrumParam[1].swell = 1.0f;
    waterFFTParam.spectrumParam[1].peakEnhancement = 1.0f;
    waterFFTParam.spectrumParam[1].shortWavesFade = 0.01f;
    waterFFTParam.spectrumParam[1].useSpectrum = true;

    waterFFTParam.spectrumParam[2].lengthScale = 64.0f;
    waterFFTParam.spectrumParam[2].tile = 32.0f;
    waterFFTParam.spectrumParam[2].scale = 0.01f;
    waterFFTParam.spectrumParam[2].windSpeed = 2.0f;
    waterFFTParam.spectrumParam[2].windDirection = 59.0f;
    waterFFTParam.spectrumParam[2].fetch = 1000.0f;
    waterFFTParam.spectrumParam[2].spreadBlend = 0.0f;
    waterFFTParam.spectrumParam[2].swell = 1.0f;
    waterFFTParam.spectrumParam[2].peakEnhancement = 1.0f;
    waterFFTParam.spectrumParam[2].shortWavesFade = 0.01f;
    waterFFTParam.spectrumParam[2].useSpectrum = false;

    waterFFTParam.spectrumParam[3].lengthScale = 32.0f;
    waterFFTParam.spectrumParam[3].tile = 16.0f;
    waterFFTParam.spectrumParam[3].scale = 0.01f;
    waterFFTParam.spectrumParam[3].windSpeed = 2.0f;
    waterFFTParam.spectrumParam[3].windDirection = 59.0f;
    waterFFTParam.spectrumParam[3].fetch = 1000.0f;
    waterFFTParam.spectrumParam[3].spreadBlend = 0.0f;
    waterFFTParam.spectrumParam[3].swell = 1.0f;
    waterFFTParam.spectrumParam[3].peakEnhancement = 1.0f;
    waterFFTParam.spectrumParam[3].shortWavesFade = 0.01f;
    waterFFTParam.spectrumParam[3].useSpectrum = false;

    GUI::WaterFFTParam::PBRWaterParam& pbr = waterFFTParam.PBRWater;
    pbr.roughness = 0.075f;
    pbr.metallic = 0.0f;
    pbr.sunIrradiance[0] = 255.0f; pbr.sunIrradiance[1] = 177.0f; pbr.sunIrradiance[2] = 82.0f; GUI::color01(pbr.sunIrradiance, 3);
    pbr.scatterColor[0] = 4.0f; pbr.scatterColor[1] = 19.0f; pbr.scatterColor[2] = 41.0f; GUI::color01(pbr.scatterColor, 3);
    pbr.bubbleColor[0] = 0.0f; pbr.bubbleColor[2] = 5.0f; pbr.bubbleColor[2] = 4.0f; GUI::color01(pbr.bubbleColor, 3);
    pbr.heightModifier = 1.0f;
    pbr.foamRoughnessModifier = 0.0f;
    pbr.bubbleDensity = 1.0f;
    pbr.wavePeakScatterStrength = 1.0f;
    pbr.scatterStrength = 1.0f;
    pbr.scatterShadowStrength = 0.5f;
    pbr.environmentLightStrength = 1.0f;

    waterFFTParam.foamParam.foamColor[0] = 0.6f; waterFFTParam.foamParam.foamColor[1] = 0.5568f; waterFFTParam.foamParam.foamColor[2] = 0.492f;
    //GUI::color01(waterFFTParam.foamParam.foamColor, 3);
    waterFFTParam.foamParam.foamBias = 1.0f;
    waterFFTParam.foamParam.foamAdd = 1.0f;
    waterFFTParam.foamParam.foamThreshold = 0.3f;
    waterFFTParam.foamParam.foamDecayRate = 0.5f;
    waterFFTParam.foamParam.foamSubtract[0] = 0.01f;
    waterFFTParam.foamParam.foamSubtract[1] = -0.02f;
    waterFFTParam.foamParam.foamSubtract[2] = -0.03f;
    waterFFTParam.foamParam.foamSubtract[3] = -0.04f;
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
    for (int i = 0; i < DEBUGSIZE; i++) {
        for (int j = 0; j < TEXTURE_ARRAY_SIZE; j++) {
            int indx = i * TEXTURE_ARRAY_SIZE + j;
            glGenVertexArrays(1, &debug[indx].vao);
            glBindVertexArray(debug[indx].vao);

            glGenBuffers(1, &debug[indx].ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, debug[indx].ebo);
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
}

void WaterFFT::drawDebugPlane(Shader *shader, glm::mat4 projection, glm::mat4 view) {

    if (hasCreateDebugPlane == false) {
        createDebugPlane();
        hasCreateDebugPlane = true;
    }
    float visibleStrength[5] = { 1234.0f, 1.0f, 1.0f, 1.0f, 1.0f };
    for (int i = 0; i < DEBUGSIZE; i++) {
        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);

        shader->setInt("Textures", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, debug[i].tex);

        shader->setFloat("visibleStrength", visibleStrength[i]);

        for (int j = 0; j < TEXTURE_ARRAY_SIZE; j++) {
            int indx = i * TEXTURE_ARRAY_SIZE + j;
            glBindVertexArray(debug[indx].vao);
            glm::mat4 m(1.0f);
            float xx = i * 2.0f;
            float zz = j * 2.0f;
            m = glm::translate(m, glm::vec3(-1.0f + xx, 12.0f, 0.0f + zz));

            shader->setMat4("model", m);
            shader->setFloat("texIndex", j);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, debug[indx].ebo);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

            glBindVertexArray(0);
        }
    }
}

void WaterFFT::createShader(std::string filename) {
    std::string vs = filename + ".vs";
    std::string fs = filename + ".fs";
    std::string tcs = filename + ".tcs";
    std::string tes = filename + ".tes";

    waterFFTShader = new ShaderT(vs, fs, "", tcs, tes);
}

void WaterFFT::createComputeShader() {
    std::string computePath = "waterFFT_InitSpectrum.sc";
    compute_InitialSpectrum = new ComputeShader(computePath);

    computePath = "waterFFT_UpdateSpectrum.sc";
    compute_UpdateSpectrum = new ComputeShader(computePath);

    computePath = "waterFFT_PackSpectrumConjugate.sc";
    compute_PackSpectrumConjugate = new ComputeShader(computePath);

    computePath = "waterFFT_FFTHorizontal.sc";
    compute_FFTHorizontal = new ComputeShader(computePath);

    computePath = "waterFFT_FFTVertical.sc";
    compute_FFTVertical = new ComputeShader(computePath);

    computePath = "waterFFT_AssembleMaps.sc";
    compute_AssembleMaps = new ComputeShader(computePath);
}

float WaterFFT::JonswapAlpha(float fetch, float windSpeed) {
    return 0.076f * std::pow(waterFFTParam.waterUniform.gravity * fetch / windSpeed / windSpeed, -0.22f);
}

float WaterFFT::JonswapPeakFrequency(float fetch, float windSpeed) {
    return 22.0f * std::pow(windSpeed * fetch / waterFFTParam.waterUniform.gravity / waterFFTParam.waterUniform.gravity, -0.33f);
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