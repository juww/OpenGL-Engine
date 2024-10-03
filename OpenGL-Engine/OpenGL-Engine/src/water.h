#pragma once
#ifndef WATER_H
#define WATER_H

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <vector>

#include "shader_m.h"

class Water {
public:

    unsigned int m_Vao, m_Ebo;
    
    std::vector<glm::vec3> m_Vertices;
    std::vector<unsigned int> m_Indices;

    glm::mat4 m_Model;
    int m_Width, m_Height;
    float m_Scale;
    
    unsigned int tvao, tebo;
    unsigned int tsizeIndices;
    unsigned int ttex, spectrumTex, displacementTex, slopeTex;

    Water();
    ~Water();

    void update();
    void setParameter(Shader* shader, float& _a, float& _f, float& _t, float& _s, float &seed, float &iter, int &waveCount, glm::vec3& cameraPos);
    void draw(Shader *shader, glm::mat4 projection, glm::mat4 view);
    void drawNormalLine(Shader* shader, glm::mat4 projection, glm::mat4 view);

    void createSpectrum(int N);
    void spectrumPlane(int N);
    void updateSpectrum(int N, float repeatTime, float frameTime);
    void horizontalFFT(int N);
    void verticalFFT(int N);
    glm::vec4 FFT(unsigned int Index, glm::vec4 input);
    void ButterflyValues(unsigned int step, unsigned int index, glm::ivec2& indices, glm::vec2& twiddle);
    void drawSpectrum(Shader* shader, glm::mat4 projection, glm::mat4 view);

    void initialize(const int& width, const int& height, const float& scale);
    
private:

    std::vector<std::vector<int> > m_IndexMap;

    void setupIndexMap();
    void setupIndices();
    void setupVectices();

    float JONSWAPSpectrum(float omega);
    float PhillipsSpectrum(float k, float L);
};

#endif // !WATER_H
