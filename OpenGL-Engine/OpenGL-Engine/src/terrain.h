#pragma once
#ifndef TERRAIN_H
#define TERRAIN_H

#include "shader_t.h"
#include "computeShader.h"

class T_Terrain {
public:
    int width, heigth;
    unsigned int vao, ebo;
    unsigned int noiseTexture, textureSize;
    bool isUpdate;
    int patchSize;
    ShaderT* shader;
    ComputeShader* noiseShader;

    T_Terrain();
    ~T_Terrain();

    void createPlane(int w, int h);
    void setShader(ShaderT* p_shader);
    void setComputeShader(ComputeShader* p_compute);
    void generateNoiseTexture(int t_size);
    void computeNoiseMap();

    void draw(glm::mat4 projection, glm::mat4 view, glm::vec3 viewPos);
};

#endif
