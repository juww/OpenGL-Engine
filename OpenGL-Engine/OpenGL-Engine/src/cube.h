#ifndef CUBE_H
#define CUBE_H

#include <glm/glm.hpp>
#include "shader_m.h"

class Cube {
public:

    glm::vec3 pos, rot, scale;
    glm::vec3 color;
    glm::mat4 model;
    unsigned int vao = 0, ebo = 0;

    void initialize() {
        model = glm::mat4(1.0f);

        pos = glm::vec3(0.0f);
        scale = glm::vec3(1.0f);

        color = glm::vec3(1.0f);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, N_INDICES * sizeof(unsigned int), &indices, GL_STATIC_DRAW);

        unsigned int vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        unsigned int sizeBuffer = N_VERTEX * (3 + 3) * sizeof(float);
        glBufferData(GL_ARRAY_BUFFER, sizeBuffer, &vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
        glEnableVertexAttribArray(0);

    }

    void localTransform() {
        model = glm::translate(model, pos);
        model = glm::scale(model, scale);
    }

    void update(const float& currentTime) {
        const float PI = 3.14159265359;

        glm::vec3 v = glm::vec3(0.0f, 10.0f, -3.0f) - pos;
        float r = glm::length(v);

        float ph = (PI / 2.0f) - (PI);
        float xz = r * glm::cos(ph);

        float th = 2.0f * PI * currentTime * -1.0f;

        float update_x = 7.0f * glm::cos(th);
        float update_z = 7.0f * glm::sin(th);
        pos = glm::vec3(update_x, 12.0f, update_z);
    }

    void draw(Shader* shader, const glm::mat4& projection, const glm::mat4& view) {
        shader->use();

        glm::mat4 m(1.0f);

        m = glm::translate(m, pos);
        m = glm::scale(m, scale);

        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        shader->setMat4("model", m);
        shader->setVec3("lightColor", color);

        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glDrawElements(GL_TRIANGLES, N_INDICES, GL_UNSIGNED_INT, (void*)0);

        glBindVertexArray(0);
    }

private:
    static const int N_VERTEX = 24;
    const float vertices[N_VERTEX * (3 + 3)] = {
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
       -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

       -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
       -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

       -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
       -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
       -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
       -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

       -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
       -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,

       -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
       -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f
    };
    static const unsigned int N_INDICES = 36;
    const unsigned int indices[N_INDICES] = {
         0,  2,  1,  0,  3,  2,
         4,  6,  5,  4,  7,  5,
         8, 10,  9,  8, 11, 10,
        15, 14, 13, 15, 13, 12,
        19, 16, 17, 19, 17, 18,
        22, 20, 21, 22, 23, 20
    };
};
#endif
