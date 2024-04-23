#ifndef CUBE_H
#define CUBE_H

#include <glm/glm.hpp>
#include "shader_m.h"

class Cube {
public:

    glm::vec3 pos, rot, scale;
    glm::mat4 model;
    unsigned int vao = 0, ebo = 0;

    void initialize() {
        model = glm::mat4(1.0f);

        model = glm::translate(model, { 1.0f, 0.0f, 0.0f });
        model = glm::scale(model, glm::vec3(0.2f));

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

    void draw(Shader* shader, const glm::mat4& projection, const glm::mat4& view) {
        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        shader->setMat4("model", model);

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
