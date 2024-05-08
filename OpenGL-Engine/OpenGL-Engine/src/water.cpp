#include "water.h"

Water::Water() {

}

Water::~Water() {

}

void Water::initialize(const int& width, const int& height) {
    m_Model = glm::mat4(1.0f);
    m_Model = glm::translate(m_Model, { -width * 0.25f, 10.0, -height * 0.25f});
    m_Model = glm::scale(m_Model, glm::vec3(0.25));
    
    m_Width = width;
    m_Height = height;

    glGenVertexArrays(1, &m_Vao);
    glBindVertexArray(m_Vao);

    setupIndexMap();
    setupIndices();
    setupVectices();

    glBindVertexArray(0);
}

void Water::setParameter(Shader *shader, float& _a, float& _f, float& _t, float& _s, float& seed, float& iter, int& waveCount, glm::vec3 &cameraPos) {
    shader->use();
    shader->setFloat("_amplitude", _a);
    shader->setFloat("_frequency", 2.0f/_f);
    shader->setFloat("_time", _t);
    shader->setFloat("_speed", _s);
    shader->setFloat("_seed", seed);
    shader->setFloat("_iter", 1.234f);
    shader->setInt("_waveCount", waveCount);
    shader->setVec3("lightDirection", glm::vec3(-0.2f, -1.0f, -0.3f));
    shader->setVec3("viewPos", cameraPos);
}

void Water::draw(Shader* shader, glm::mat4 projection, glm::mat4 view) {

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    shader->use();

    shader->setMat4("model", m_Model);
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);

    glBindVertexArray(m_Vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo);
    glDrawElements(GL_TRIANGLE_STRIP, m_Indices.size(), GL_UNSIGNED_INT, (void*)0);

    glBindVertexArray(0);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Water::setupIndexMap() {
    m_IndexMap.clear();
    m_IndexMap.resize(m_Height);
    int idx = 0;
    for (int i = 0; i < m_Height; i++) {
        for (int j = 0; j < m_Width + 2; j++) {
            m_IndexMap[i].push_back(idx);
            if (i == m_Height - 1) {
                if (j == 0 || j == m_Width + 1) {
                    m_IndexMap[i][j] = -1;
                    continue;
                }
            }
            idx++;
        }
    }
}

void Water::setupIndices() {

    glGenBuffers(1, &m_Ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo);
    for (int i = 0; i < m_Height - 1; i++) {
        for (int j = 0; j < m_Width + 2; j++) {
            m_Indices.push_back(m_IndexMap[i][j]);
            if (j > 0 && j < m_Width + 1) {
                m_Indices.push_back(m_IndexMap[i + 1][j]);
            }
        }
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), &m_Indices.at(0), GL_STATIC_DRAW);
}

void Water::setupVectices() {
    for (int i = 0; i < m_Height; i++) {
        for (int j = 0; j < m_Width + 2; j++) {
            if (m_IndexMap[i][j] == -1) continue;
            m_Vertices.push_back(glm::vec3(j - 1, 0.0, i));
            if (j == 0) {
                m_Vertices[m_Vertices.size() - 1].x = j;
                continue;
            }
            if (j == m_Width + 1) {
                m_Vertices[m_Vertices.size() - 1] = glm::vec3(j - 2, 0.0, i + 1);
                continue;
            }
        }
    }
    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    unsigned int sizeBuffer = m_Vertices.size() * 3 * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, sizeBuffer, &m_Vertices.at(0), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glEnableVertexAttribArray(0);

}