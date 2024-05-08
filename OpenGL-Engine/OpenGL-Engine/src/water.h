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

    Water();
    ~Water();

    void update();
    void setParameter(Shader* shader, float& _a, float& _f, float& _t, float& _s, float &seed, float &iter, int &waveCount, glm::vec3& cameraPos);
    void draw(Shader *shader, glm::mat4 projection, glm::mat4 view);

    void initialize(const int& width, const int & height);
    
private:

    std::vector<std::vector<int> > m_IndexMap;

    void setupIndexMap();
    void setupIndices();
    void setupVectices();
};

#endif // !WATER_H
