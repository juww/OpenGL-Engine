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
    float lengthInv;
    glm::vec3 pos, rot, scale;
    glm::mat4 model;
    unsigned int vao = 0, ebo = 0;
    unsigned int tex;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    const float PI = 3.14159265359;

    Sphere(const int& n, const float& r) {
        length = n;
        radius = r;
        model = glm::mat4(1.0f);
        pos = rot = scale = glm::vec3(0.0f);
        vertices.clear();
        indices.clear();
        lengthInv = 1.0f / radius;

        pos = glm::vec3(3.0f);

        model = glm::translate(model, pos);
    }

    void createHemisphere() {
        for (int i = 0; i <= length; i++) {
            float ph = (PI / 2.0f) - (PI * (float)((float)i / (float)length));
            for (int j = 0; j <= length; j++) {
                float th = 2.0f * PI * (float)((float)j / (float)length);

                float x = (radius * glm::cos(ph)) * glm::cos(th);
                float y = (radius * glm::sin(ph));
                float z = (radius * glm::cos(ph)) * glm::sin(th);
                //pos
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                //normal
                vertices.push_back(x * lengthInv);
                vertices.push_back(y * lengthInv);
                vertices.push_back(z * lengthInv);
                //texcoord
                float tx = (float)j / length;
                float ty = (float)i / length;
                vertices.push_back(tx);
                vertices.push_back(ty);

                if (i == 0 || i == length) break;
            }
        }
        printf("total = %d\n", vertices.size());
        printf("vertices = %d\n", vertices.size() / 8);
        for (int i = 0; i < length - 1; i++) {
            int k1 = i * (length + 1);
            int k2 = (i + 1) * (length + 1);
            for (int j = 1; j <= length + 1; j++) {
                //int next = (j + 1 > length + 1 ? 1 : j + 1);
                int next = j + 1;
                if (i == 0) {
                    //printf("%d %d %d\n", 0, j + k1, next + k1);
                    indices.push_back(0);
                    indices.push_back(j + k1);
                    indices.push_back(next + k1);
                }
                if (i == length - 2) {
                    //printf("%d %d %d\n", j + k1, (vertices.size() / 8) - 1, next + k1);
                    indices.push_back(j + k1);
                    indices.push_back((vertices.size() / 8) - 1);
                    indices.push_back(next + k1);
                    continue;
                }
                printf("%d %d %d\n", j + k1, j + k2, next + k1);
                indices.push_back(j + k1);
                indices.push_back(j + k2);
                indices.push_back(next + k1);

                indices.push_back(next + k1);
                indices.push_back(j + k2);
                indices.push_back(next + k2);
            }
        }
        setbuffer();
    }

    void draw(Shader* shader, const glm::mat4& projection, const glm::mat4& view, glm::vec3& cameraPos, const float &_time) {
        shader->use();

        glm::mat4 m = glm::rotate(model, _time, glm::vec3(0.0f, 1.0f, 0.0f));

        shader->setMat4("model", m);
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);

        shader->setVec3("lightDirection", glm::vec3(1.0f, -1.0f, 1.0f));
        shader->setVec3("viewPos", cameraPos);
        shader->setInt("Textures", 0);

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBindVertexArray(vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        //glPointSize(10);
        //glDrawArrays(GL_POINTS, 0, vertices.size() / 8);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);

        glBindVertexArray(0);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    ~Sphere() {

    }
private:
    void setbuffer() {

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices.at(0), GL_STATIC_DRAW);

        unsigned int vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        int VSIZE = (3 + 3 + 2);
        unsigned int sizeBuffer = vertices.size() * sizeof(float);
        glBufferData(GL_ARRAY_BUFFER, sizeBuffer, &vertices.at(0), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * VSIZE, (void*)0);
        glEnableVertexAttribArray(0);
        
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * VSIZE, (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * VSIZE, (void*)(sizeof(float) * 6));
        glEnableVertexAttribArray(2);

        loadTexture();

        glBindVertexArray(0);
    }

    void loadTexture() {
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int w, h, nrChannels;
        unsigned char* data = stbi_load(FileSystem::getPath("res/textures/earth2048.bmp").c_str(), &w, &h, &nrChannels, 4);
        printf("check data\n");
        if (data) {
            printf("data exist\n");
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        stbi_image_free(data);
    }
};

#endif