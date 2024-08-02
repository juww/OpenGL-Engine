#ifndef SPHERE_H
#define SPHERE_H

#include <glm/glm.hpp>
#include <vector>
#include "shader_m.h"
#include <math.h>

class Sphere {
public:
    int length;
    float radius;
    glm::vec3 pos, rot, scale;
    glm::mat4 model;
    unsigned int vao = 0, ebo = 0;
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;

    const float PI = 3.14159265359;

    Sphere(const int& n, const float& r) {
        length = n;
        radius = r;
        model = glm::mat4(1.0);
        vertices.clear();
        for (int i = 0; i <= length; i++) {
            float ph = (PI / 2.0f) - (PI * (float)((float)i / (float)length));
            for (int j = 0; j <= length; j++) {
                float th = 2.0f * PI * (float)((float)j / (float)length);

                float x = (r * glm::cos(ph)) * glm::cos(th);
                float y = (r * glm::sin(ph));
                float z = (r * glm::cos(ph)) * glm::sin(th);
                glm::vec3 vec(x, y, z);
                vertices.push_back(vec);
            }
        }

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        printf("vertices.size() = %d\n", vertices.size());
        /*for (int i = 0; i < vertices.size(); i++) {
            indices.push_back(i);
        }*/
        // 11 x 11 = 121

        for (int i = 0; i < length; i++) {
            int k1 = i * (length + 1);
            int k2 = (i+1) * (length + 1);
            for (int j = 0; j < length; j++,k1++,k2++) {
                
                if (i != 0) {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }
                if (i != length - 1) {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices.at(0), GL_STATIC_DRAW);

        unsigned int vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        unsigned int sizeBuffer = vertices.size() * 3 * sizeof(float);
        glBufferData(GL_ARRAY_BUFFER, sizeBuffer, &vertices.at(0), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    void draw(Shader* shader, const glm::mat4& projection, const glm::mat4& view) {
        shader->use();

        shader->setMat4("model", model);
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);


        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);

        glBindVertexArray(0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    ~Sphere() {

    }
};

#endif