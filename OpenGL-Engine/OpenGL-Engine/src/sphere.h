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
    int countVertex;
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
        countVertex = 0;
        pos = glm::vec3(3.0f);

        model = glm::translate(model, pos);
    }

    void createHemisphere() {
        for (int i = 0; i <= length; i++) {
            float ph = (PI / 2.0f) - (PI * (float)((float)i / (float)length));
            float xz = radius * glm::cos(ph);
            float y = radius * glm::sin(ph);
            for (int j = 0; j <= length; j++) {
                float th = 2.0f * PI * (float)((float)j / (float)length);

                float x = xz * glm::cos(th);
                float z = xz * glm::sin(th);
                
                float tx = (float)j / length;
                float ty = (float)i / length;
                addVertex(x, y, z, tx, ty);
            }
        }
        printf("total = %d\n", vertices.size());
        printf("vertices = %d\n", vertices.size() / 8);
        for (int i = 0; i < length; i++) {
            int k1 = i * (length + 1);
            int k2 = (i + 1) * (length + 1);
            for (int j = 0; j < length; j++) {
                int next = j + 1;
                if (i != 0) {
                    indices.push_back(j + k1);
                    indices.push_back(j + k2);
                    indices.push_back(next + k1);
                }
                if (i != length - 1) {
                    indices.push_back(next + k1);
                    indices.push_back(j + k2);
                    indices.push_back(next + k2);
                }
            }
        }
        setbuffer();
    }

    void icosphere(const int& lvl) {
        const float atn = glm::atan(1.0f / 2.0f);
        const float hAngle = PI / 180.0f * 72.0f;

        float h1 = -PI / 2.0f - hAngle / 2.0f;
        float h2 = -PI / 2.0f;

        std::vector<unsigned int> tempIndices;

        float x = 0.0f;
        float y = radius;
        float z = 0.0f;
        
        addVertex(x, y, z, 0.0f, 0.0f);
        for (int i = 1; i <= 5; i++) {
            float xz = radius * glm::cos(atn);
            int next = i + 1 > 5 ? 1 : i + 1;
            x = xz * glm::cos(h1);
            y = radius * glm::sin(atn);
            z = xz * glm::sin(h1);
            addVertex(x, y, z, 0.0f, 0.0f);
            tempIndices.push_back(0);
            tempIndices.push_back(i);
            tempIndices.push_back(next);

            tempIndices.push_back(i);
            tempIndices.push_back(i + 5);
            tempIndices.push_back(next);
            h1 += hAngle;
        }
        for (int i = 1; i <= 5; i++) {
            float xz = radius * glm::cos(atn);
            int next = i + 1 > 5 ? 1 : i + 1;
            x = xz * glm::cos(h2);
            y = -radius * glm::sin(atn);
            z = xz * glm::sin(h2);
            addVertex(x, y, z, 0.0f, 0.0f);

            tempIndices.push_back(i + 5);
            tempIndices.push_back(next + 5);
            tempIndices.push_back(next);

            tempIndices.push_back(i + 5);
            tempIndices.push_back(11);
            tempIndices.push_back(next + 5);
            h2 += hAngle;
        }
        x = 0.0f;
        y = -radius;
        z = 0.0f;
        addVertex(x, y, z, 0.0f, 0.0f);

        int baseIndexSize = tempIndices.size();
        for (int i = 0; i < baseIndexSize; i+=3 ) {   
            subDivisionTriangle(lvl, tempIndices[i], tempIndices[i + 1], tempIndices[i + 2]);
        }
        printf("count Vertex: %d\n", countVertex);
        printf("size vertices = %d\n", vertices.size());
        printf("%d\n", vertices.size()/8);
        printf("size indices = %d\n", indices.size());
        printf("count triangle = %d\n", indices.size() / 3);
        setbuffer();
    }

    void cubesphere(const int& lvl) {
        const float rad = acos(-1) / 180.0f;

        glm::vec3 n1 = glm::vec3(-glm::sin(rad * 45.0f), 0.0f, glm::cos(rad * 45.0f));
        glm::vec3 n2 = glm::vec3(-glm::sin(rad * -45.0f), -glm::cos(rad * -45.0f), 0.0f);

        glm::vec3 v = glm::cross(n1, n2);
        float scale = radius / sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
        v *= scale;
        float inv[2] = { 1.0f, -1.0f };
        printf("%f %f %f\n", n1.x, n1.y, n1.z);
        printf("%f %f %f\n", n2.x, n2.y, n2.z);
        printf("v = %f %f %f\n", v.x, v.y, v.z);
        int indx = 0;
        for (int i = 0; i < 2; i++) {
            float x = v.x * inv[i];
            for (int j = 0; j < 2; j++) {
                float y = v.y * inv[j];
                for (int k = 0; k < 2; k++) {
                    float z = v.z * inv[k];
                    addVertex(x, y, z);
                    printf("%d ---  %f %f %f\n", indx++, x, y, z);
                }
            }
        }

        unsigned int baseRectangle[24] = {
            0, 4, 6, 2,
            1, 0, 2, 3,
            5, 4, 0, 1,
            2, 6, 7, 3,
            4, 5, 7, 6,
            5, 1, 3, 7
        };
        for (int i = 0; i < 24; i += 4) {
            subDivisionRectangle(lvl, baseRectangle[i], baseRectangle[i + 1], baseRectangle[i + 2], baseRectangle[i + 3]);
        }
        setbuffer();
    }

    void draw(Shader* shader, const glm::mat4& projection, const glm::mat4& view, glm::vec3& cameraPos, const float &_time) {
        shader->use();

        glm::mat4 m = glm::rotate(model, _time, glm::vec3(0.0f, 1.0f, 0.0f));

        shader->setMat4("model", model);
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

    int addVertex(const float &x, const float &y, const float &z, const float tx = 0.0f, const float ty = 0.0f) {
        //pos
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
        //normal
        vertices.push_back(x * lengthInv);
        vertices.push_back(y * lengthInv);
        vertices.push_back(z * lengthInv);
        //texcoord
        vertices.push_back(tx);
        vertices.push_back(ty);
        countVertex++;

        return countVertex - 1;
    }

    void subDivisionTriangle(int lvl, unsigned int indx1, unsigned int indx2, unsigned int indx3) {
        if (lvl == 0) {
            indices.push_back(indx1);
            indices.push_back(indx2);
            indices.push_back(indx3);
            return;
        }
        int p1 = indx1 * 8;
        int p2 = indx2 * 8;
        int p3 = indx3 * 8;

        float v1[3] = { vertices[p1],vertices[p1 + 1], vertices[p1 + 2] };
        float v2[3] = { vertices[p2],vertices[p2 + 1], vertices[p2 + 2] };
        float v3[3] = { vertices[p3],vertices[p3 + 1], vertices[p3 + 2] };

        float newV1[3] = { 0.0f, 0.0f, 0.0f };
        float newV2[3] = { 0.0f, 0.0f, 0.0f };
        float newV3[3] = { 0.0f, 0.0f, 0.0f };

        computeHalfVertex(v1, v2, newV1);
        computeHalfVertex(v2, v3, newV2);
        computeHalfVertex(v3, v1, newV3);

        int idx1 = addVertex(newV1[0], newV1[1], newV1[2]);
        int idx2 = addVertex(newV2[0], newV2[1], newV2[2]);
        int idx3 = addVertex(newV3[0], newV3[1], newV3[2]);

        subDivisionTriangle(lvl - 1, indx1, idx1, idx3);
        subDivisionTriangle(lvl - 1, indx2, idx2, idx1);
        subDivisionTriangle(lvl - 1, indx3, idx3, idx2);
        subDivisionTriangle(lvl - 1, idx1, idx2, idx3);
    }

    void subDivisionRectangle(int lvl, unsigned int indx1, unsigned int indx2, unsigned int indx3, unsigned int indx4) {
        if (lvl == 0) {
            indices.push_back(indx1);
            indices.push_back(indx2);
            indices.push_back(indx4);
            
            indices.push_back(indx3);
            indices.push_back(indx4);
            indices.push_back(indx2);
            return;
        }
        int p1 = indx1 * 8;
        int p2 = indx2 * 8;
        int p3 = indx3 * 8;
        int p4 = indx4 * 8;

        float v1[3] = { vertices[p1],vertices[p1 + 1], vertices[p1 + 2] };
        float v2[3] = { vertices[p2],vertices[p2 + 1], vertices[p2 + 2] };
        float v3[3] = { vertices[p3],vertices[p3 + 1], vertices[p3 + 2] };
        float v4[3] = { vertices[p4],vertices[p4 + 1], vertices[p4 + 2] };

        float newV1[3] = { 0.0f, 0.0f, 0.0f };
        float newV2[3] = { 0.0f, 0.0f, 0.0f };
        float newV3[3] = { 0.0f, 0.0f, 0.0f };
        float newV4[3] = { 0.0f, 0.0f, 0.0f };
        float newV5[3] = { 0.0f, 0.0f, 0.0f };

        computeHalfVertex(v1, v2, newV1);
        computeHalfVertex(v2, v3, newV2);
        computeHalfVertex(v3, v4, newV3);
        computeHalfVertex(v4, v1, newV4);
        computeHalfVertex(newV1, newV3, newV5);

        int idx1 = addVertex(newV1[0], newV1[1], newV1[2]);
        int idx2 = addVertex(newV2[0], newV2[1], newV2[2]);
        int idx3 = addVertex(newV3[0], newV3[1], newV3[2]);
        int idx4 = addVertex(newV4[0], newV4[1], newV4[2]);
        int idx5 = addVertex(newV5[0], newV5[1], newV5[2]);

        subDivisionRectangle(lvl - 1, indx1, idx1, idx5, idx4);
        subDivisionRectangle(lvl - 1, idx1, indx2, idx2, idx5);
        subDivisionRectangle(lvl - 1, idx5, idx2, indx3, idx3);
        subDivisionRectangle(lvl - 1, idx4, idx5, idx3, indx4);
    }

    void computeHalfVertex(const float v1[3], const float v2[3], float newV[3])
    {
        newV[0] = v1[0] + v2[0];    // x
        newV[1] = v1[1] + v2[1];    // y
        newV[2] = v1[2] + v2[2];    // z
        float scale = radius / sqrtf((newV[0] * newV[0]) + (newV[1] * newV[1]) + (newV[2] * newV[2]));
        newV[0] *= scale;
        newV[1] *= scale;
        newV[2] *= scale;
    }

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