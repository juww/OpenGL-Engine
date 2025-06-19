#include "terrain.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <glm/gtx/transform.hpp>

T_Terrain::T_Terrain() {
    ebo = 0;
    vao = 0;
    width = heigth = 0;
    patchSize = 0;
    shader = nullptr;
    noiseShader = nullptr;
}

T_Terrain ::~T_Terrain() {

}

void T_Terrain::createPlane(int w, int h) {

    width = w;
    heigth = h;
    std::vector<glm::vec3> patchPos;

    int nn = heigth + 1, mm = width + 1;
    for (int i = 0; i < nn; i++) {
        for (int j = 0; j < mm; j++) {
            patchPos.push_back(glm::vec3(j, 0.0f, i));
        }
    }

    std::vector<unsigned int> patchIndx;
    for (int i = 0; i < heigth; i++) {
        for (int j = 0; j < width; j++) {
            //quad
            patchIndx.push_back(i * mm + j);
            patchIndx.push_back(i * mm + (j + 1));
            patchIndx.push_back((i + 1) * mm + (j + 1));
            patchIndx.push_back((i + 1) * mm + j);
        }
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glPatchParameteri(GL_PATCH_VERTICES, 4);

    patchSize = patchIndx.size();
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, patchSize * sizeof(unsigned int), &patchIndx[0], GL_STATIC_DRAW);

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, patchPos.size() * sizeof(glm::vec3), &patchPos[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void T_Terrain::computeNoiseMap() {
    noiseShader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glBindImageTexture(0, noiseTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glDispatchCompute((textureSize / 8), (textureSize / 8), 1);
    // make sure writing to image has finished before read
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void T_Terrain::draw(glm::mat4 projection, glm::mat4 view, glm::vec3 viewPos) {

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glm::mat4 m(1.0f);
    m = glm::translate(m, { -width, 6.0f, 0.0f });

    shader->use();
    shader->setFloat("width", width);
    shader->setFloat("heigth", heigth);

    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setMat4("model", m);

    glBindVertexArray(vao);

    shader->setInt("heightMap", 0);
    shader->setVec3("viewPos", viewPos);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_PATCHES, patchSize, GL_UNSIGNED_INT, (void*)0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void T_Terrain::generateNoiseTexture(int t_size) {

    textureSize = t_size;
    glGenTextures(1, &noiseTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureSize, textureSize, 0, GL_RGBA, GL_FLOAT, NULL);

}

void T_Terrain::setComputeShader(ComputeShader *p_compute) {
    noiseShader = p_compute;
}

void T_Terrain::setShader(ShaderT* p_shader) {
    shader = p_shader;
}