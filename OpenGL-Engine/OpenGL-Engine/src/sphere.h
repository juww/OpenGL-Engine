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
    float widthTex, heightTex;
    unsigned int tex, cubeTex;
    std::vector<float> vertices;
    std::vector<std::string> texturePaths;
    std::vector<unsigned int> indices;
    std::vector<float> faces;

    const float PI = 3.14159265359;

    Sphere(const int& n, const float& r) {
        length = n;
        radius = r;
        model = glm::mat4(1.0f);
        pos = rot = scale = glm::vec3(0.0f);
        vertices.clear();
        indices.clear();
        texturePaths.clear();
        widthTex = heightTex = 0.0f;
        lengthInv = 1.0f / radius;
        countVertex = 0;
        tex = 0, cubeTex = 0;
        pos = glm::vec3(10.0f);

        model = glm::translate(model, pos);
        getTexturePath();
    }

    void createHemisphere() {
        loadTexture(texturePaths[2]);
        for (int i = 0; i <= length; i++) {
            float ph = (PI / 2.0f) - (PI * (float)((float)i / (float)length));
            float xz = radius * glm::cos(ph);
            float y = radius * glm::sin(ph);
            for (int j = 0; j <= length; j++) {
                float th = 2.0f * PI * (float)((float)j / (float)length) * -1.0f;

                float x = xz * glm::cos(th);
                float z = xz * glm::sin(th);
                
                float tx = (float)j / length;
                float ty = (float)i / length;
                addVertex(x, y, z, tx, ty);
            }
        }

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

        loadTexture(texturePaths[1]);

        float invS = widthTex / 11.0f;
        float invT = heightTex / 3.0f;
        float s = 186.0f / widthTex;
        float t = 322.0f / heightTex;

        std::vector<unsigned int> baseTriangle;

        float x = 0.0f;
        float y = radius;
        float z = 0.0f;
        
        for (int i = 0; i < 5; i++) {
            float xz = radius * glm::cos(atn);
            float yy = radius * glm::sin(atn);

            float hor1 = h1 + (hAngle * i);
            glm::vec3 v1(xz * glm::cos(hor1), yy, xz * glm::sin(hor1));
            hor1 = h1 + (hAngle * (i + 1));
            glm::vec3 v2(xz * glm::cos(hor1), yy, xz * glm::sin(hor1));

            float hor2 = h2 + (hAngle * i);
            glm::vec3 v3(xz * glm::cos(hor2), -yy, xz * glm::sin(hor2));
            hor2 = h2 + (hAngle * (i + 1));
            int cur = i * 2;
            int mid = (i * 2 + 1);
            int next = ((i + 1) * 2);
            int next2 = ((i + 1) * 2 + 1);
            glm::vec3 v4(xz * glm::cos(hor2), -yy, xz * glm::sin(hor2));
            unsigned int p1 = addVertex(x, y, z, float((i * 2) + 1) * s, 0.0f);
            unsigned int p2 = addVertex(v1.x, v1.y, v1.z, float(i * 2) * s, t);
            unsigned int p3 = addVertex(v2.x, v2.y, v2.z, float((i + 1) * 2) * s, t);
            baseTriangle.push_back(p1);
            baseTriangle.push_back(p2);
            baseTriangle.push_back(p3);

            p1 = addVertex(v3.x, v3.y, v3.z, float((i * 2) + 1) * s, 2.0f * t);
            p2 = addVertex(v2.x, v2.y, v2.z, float((i + 1) * 2) * s, t);
            p3 = addVertex(v1.x, v1.y, v1.z, float(i * 2) * s, t);
            baseTriangle.push_back(p1);
            baseTriangle.push_back(p2);
            baseTriangle.push_back(p3);

            p1 = addVertex(v2.x, v2.y, v2.z, float((i + 1) * 2) * s, t);
            p2 = addVertex(v3.x, v3.y, v3.z, float((i * 2) + 1) * s, 2.0f * t);
            p3 = addVertex(v4.x, v4.y, v4.z, float(((i + 1) * 2) + 1) * s, 2.0f * t);
            baseTriangle.push_back(p1);
            baseTriangle.push_back(p2);
            baseTriangle.push_back(p3);

            p1 = addVertex(x, -y, z, float((i + 1) * 2) * s, 3.0f * t);
            p2 = addVertex(v4.x, v4.y, v4.z, float(((i + 1) * 2) + 1) * s, 2.0f * t);
            p3 = addVertex(v3.x, v3.y, v3.z, float((i * 2) + 1) * s, 2.0f * t);
            baseTriangle.push_back(p1);
            baseTriangle.push_back(p2);
            baseTriangle.push_back(p3);
        }

        int baseIndexSize = baseTriangle.size();
        for (int i = 0; i < baseIndexSize; i+=3) {   
            subDivisionTriangle(lvl, baseTriangle[i], baseTriangle[i + 1], baseTriangle[i + 2]);
        }
        setbuffer();
    }

    void cubesphere(const int& lvl) {
        const float rad = acos(-1) / 180.0f;

        glm::vec3 n1 = glm::vec3(-glm::sin(rad * 45.0f), 0.0f, glm::cos(rad * 45.0f));
        glm::vec3 n2 = glm::vec3(-glm::sin(rad * -45.0f), -glm::cos(rad * -45.0f), 0.0f);

        // loadTexture(texturePaths[3]);
        loadTextureCubemap("res/textures/cubemap/");
        glm::vec3 v = glm::cross(n1, n2);
        float scale = radius / sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
        v *= scale;
        float inv[2] = { 1.0f, -1.0f };
        int indx = 0;
        for (int i = 0; i < 2; i++) {
            float x = v.x * inv[i];
            for (int j = 0; j < 2; j++) {
                float y = v.y * inv[j];
                for (int k = 0; k < 2; k++) {
                    float z = v.z * inv[k];
                    addVertex(x, y, z);
                    faces.push_back(0);
                    printf("%d: %f %f %f\n", indx++, x, y, z);
                }
            }
        }

        unsigned int baseRectangle[24] = {
            1, 0, 2, 3,         // right
            4, 5, 7, 6,         // left
            1, 5, 4, 0,         // top
            2, 6, 7, 3,         // bottom
            0, 4, 6, 2,         // front
            5, 1, 3, 7,         // back
        };
        bool vis[8];
        memset(vis, false, sizeof(vis));
        for (int i = 0; i < 24; i += 4) {
            int indx1 = baseRectangle[i], indx2 = baseRectangle[i + 1], indx3 = baseRectangle[i + 2], indx4 = baseRectangle[i + 3];
            int face = i / 4;
            indx1 = isDublicateVertex(indx1, vis, face);
            indx2 = isDublicateVertex(indx2, vis, face);
            indx3 = isDublicateVertex(indx3, vis, face);
            indx4 = isDublicateVertex(indx4, vis, face);
            faces[indx1] = faces[indx2] = faces[indx3] = faces[indx4] = face;
            printf("%d %d %d %d\n", indx1, indx2, indx3, indx4);
            vertices[(indx1 * 8) + 6] = 1.0f; vertices[(indx1 * 8) + 7] = 0.0f;
            vertices[(indx2 * 8) + 6] = 0.0f; vertices[(indx2 * 8) + 7] = 0.0f;
            vertices[(indx3 * 8) + 6] = 0.0f; vertices[(indx3 * 8) + 7] = 1.0f;
            vertices[(indx4 * 8) + 6] = 1.0f; vertices[(indx4 * 8) + 7] = 1.0f;
            subDivisionRectangle(lvl, indx1, indx2, indx3, indx4, face);
        }
        setbuffer();
    }

    void draw(Shader* shader, const glm::mat4& projection, const glm::mat4& view, glm::vec3& cameraPos, const float &_time, const unsigned int skybox) {
        shader->use();

        glm::mat4 m = glm::rotate(model, _time, glm::vec3(0.0f, 1.0f, 0.0f));

        shader->setMat4("model", m);
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);

        shader->setVec3("lightDirection", glm::vec3(1.0f, -1.0f, 1.0f));
        shader->setVec3("viewPos", cameraPos);

        glBindVertexArray(vao);

        if (tex) {
            shader->setInt("Textures", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex);
        }

        if (cubeTex) {
            shader->setInt("CubeTextures", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, cubeTex);
        }

        shader->setInt("skybox", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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

        float ts1[2] = { vertices[p1 + 6], vertices[p1 + 7] };
        float ts2[2] = { vertices[p2 + 6], vertices[p2 + 7] };
        float ts3[2] = { vertices[p3 + 6], vertices[p3 + 7] };

        float newt1[2] = { 0.0f, 0.0f };
        float newt2[2] = { 0.0f, 0.0f };
        float newt3[2] = { 0.0f, 0.0f };

        computeHalfVertex(v1, v2, newV1);
        computeHalfVertex(v2, v3, newV2);
        computeHalfVertex(v3, v1, newV3);

        computeHalfTexcoord(ts1, ts2, newt1);
        computeHalfTexcoord(ts2, ts3, newt2);
        computeHalfTexcoord(ts3, ts1, newt3);

        int idx1 = addVertex(newV1[0], newV1[1], newV1[2], newt1[0], newt1[1]);
        int idx2 = addVertex(newV2[0], newV2[1], newV2[2], newt2[0], newt2[1]);
        int idx3 = addVertex(newV3[0], newV3[1], newV3[2], newt3[0], newt3[1]);

        subDivisionTriangle(lvl - 1, indx1, idx1, idx3);
        subDivisionTriangle(lvl - 1, indx2, idx2, idx1);
        subDivisionTriangle(lvl - 1, indx3, idx3, idx2);
        subDivisionTriangle(lvl - 1, idx1, idx2, idx3);
    }

    void subDivisionRectangle(int lvl, unsigned int indx1, unsigned int indx2, unsigned int indx3, unsigned int indx4, unsigned int face) {
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

        float ts1[2] = { vertices[p1 + 6], vertices[p1 + 7] };
        float ts2[2] = { vertices[p2 + 6], vertices[p2 + 7] };
        float ts3[2] = { vertices[p3 + 6], vertices[p3 + 7] };
        float ts4[2] = { vertices[p4 + 6], vertices[p4 + 7] };

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

        float newt1[2] = { 0.0f, 0.0f };
        float newt2[2] = { 0.0f, 0.0f };
        float newt3[2] = { 0.0f, 0.0f };
        float newt4[2] = { 0.0f, 0.0f };
        float newt5[2] = { 0.0f, 0.0f };

        computeHalfTexcoord(ts1, ts2, newt1);
        computeHalfTexcoord(ts2, ts3, newt2);
        computeHalfTexcoord(ts3, ts4, newt3);
        computeHalfTexcoord(ts4, ts1, newt4);
        computeHalfTexcoord(newt1, newt3, newt5);

        int idx1 = addVertex(newV1[0], newV1[1], newV1[2], newt1[0], newt1[1]); faces.push_back(face);
        int idx2 = addVertex(newV2[0], newV2[1], newV2[2], newt2[0], newt2[1]); faces.push_back(face);
        int idx3 = addVertex(newV3[0], newV3[1], newV3[2], newt3[0], newt3[1]); faces.push_back(face);
        int idx4 = addVertex(newV4[0], newV4[1], newV4[2], newt4[0], newt4[1]); faces.push_back(face);
        int idx5 = addVertex(newV5[0], newV5[1], newV5[2], newt5[0], newt5[1]); faces.push_back(face);

        subDivisionRectangle(lvl - 1, indx1, idx1, idx5, idx4, face);
        subDivisionRectangle(lvl - 1, idx1, indx2, idx2, idx5, face);
        subDivisionRectangle(lvl - 1, idx5, idx2, indx3, idx3, face);
        subDivisionRectangle(lvl - 1, idx4, idx5, idx3, indx4, face);
    }

    int isDublicateVertex(int indx, bool vis[], int face) {
        if(vis[indx]==false){
            vis[indx] = true;
            return indx;
        }
        int idx = indx * 8;
        int ret = addVertex(vertices[idx], vertices[idx + 1], vertices[idx + 2]);
        faces.push_back(face);
        return ret;
    }

    void computeHalfTexcoord(const float t1[2], const float t2[2], float newTex[2]) {
        newTex[0] = (t1[0] + t2[0]) / 2.0f;
        newTex[1] = (t1[1] + t2[1]) / 2.0f;
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

        printf("vertex count: %d\n", countVertex);
        printf("indices size: %d\n", indices.size());

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

        if (faces.size() != 0) {
            unsigned int vboFace;
            glGenBuffers(1, &vboFace);
            glBindBuffer(GL_ARRAY_BUFFER, vboFace);
            glBufferData(GL_ARRAY_BUFFER, faces.size() * sizeof(unsigned int), &faces.at(0), GL_STATIC_DRAW);
        }
        
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
        glEnableVertexAttribArray(3);

        printf("face size: %d\n", faces.size());

        glBindVertexArray(0);
    }

    void getTexturePath() {
        texturePaths.push_back("res/textures/earth2048.bmp");
        texturePaths.push_back("res/textures/icosa_earth.bmp");
        texturePaths.push_back("res/textures/nzrzP.png");
        texturePaths.push_back("res/textures/grid512.bmp");
    }

    void loadTexture(std::string &path) {
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int w, h, nrChannels;
        unsigned char* data = stbi_load(FileSystem::getPath(path).c_str(), &w, &h, &nrChannels, 4);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        widthTex = (float)w;
        heightTex = (float)h;
        stbi_image_free(data);
    }

    void loadTextureCubemap(std::string prefixPath) {
        glGenTextures(1, &cubeTex);
        glBindTexture(GL_TEXTURE_2D_ARRAY, cubeTex);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA, 512, 512, 6);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 512, 512, 6, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

        for (int i = 0; i < 6; i++) {
            int w, h, nrChannels;
            std::string texturePath = prefixPath + "cubemap" + std::to_string(i) + ".bmp";
            unsigned char* data = stbi_load(FileSystem::getPath(texturePath).c_str(), &w, &h, &nrChannels, 4);
            if (data) {
                printf("asd\n");
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
            }
            stbi_image_free(data);
        }
    }
};

#endif