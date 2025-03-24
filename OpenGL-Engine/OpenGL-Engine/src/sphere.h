#ifndef SPHERE_H
#define SPHERE_H

#include <glm/glm.hpp>
#include <vector>
#include <math.h>
#include <algorithm>

#include "shader_m.h"
#include "material.h"
#include "GUI.h"

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
    std::map<unsigned int, bool> duplicatedVertex;
    unsigned int BUFFER_SIZE;
    Materials materials;

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
        pos = glm::vec3(0.0f, 10.0f, -3.0f);
        BUFFER_SIZE = 14;
        duplicatedVertex.clear();
        
        model = glm::translate(model, pos);
        getTexturePath();
    }

    void createHemisphere() {
        //loadTexture(texturePaths[2]);
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
                    setTangentAndBitangent(j + k1, j + k2, next + k1);
                } 
                if (i != length - 1) {
                    indices.push_back(next + k1);
                    indices.push_back(j + k2);
                    indices.push_back(next + k2);
                    setTangentAndBitangent(next + k1, j + k2, next + k2);
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

        //loadTexture(texturePaths[2]);

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
            glm::vec3 v4(xz * glm::cos(hor2), -yy, xz * glm::sin(hor2));

            float t0[2] = { 0.0f, 0.0f };
            float t1[2] = { 0.0f, 0.0f };
            float t2[2] = { 0.0f, 0.0f };
            float t3[2] = { 0.0f, 0.0f };
            float t4[2] = { 0.0f, 0.0f };
            float pos0[3] = { x, y, z };
            float pos1[3] = { v1.x, v1.y, v1.z };
            float pos2[3] = { v2.x, v2.y, v2.z };
            float pos3[3] = { v3.x, v3.y, v3.z };
            float pos4[3] = { v4.x, v4.y, v4.z };
            computeTexcoordByNewPosition(t0, pos0);
            computeTexcoordByNewPosition(t1, pos1);
            computeTexcoordByNewPosition(t2, pos2);
            computeTexcoordByNewPosition(t3, pos3);
            computeTexcoordByNewPosition(t4, pos4);

            unsigned int p1 = addVertex(x, y, z, t0[0], t0[1]);
            unsigned int p2 = addVertex(v1.x, v1.y, v1.z, t1[0], t1[1]);
            unsigned int p3 = addVertex(v2.x, v2.y, v2.z, t2[0], t2[1]);
            baseTriangle.push_back(p1);
            baseTriangle.push_back(p2);
            baseTriangle.push_back(p3);

            p1 = addVertex(v3.x, v3.y, v3.z, t3[0], t3[1]);
            p2 = addVertex(v2.x, v2.y, v2.z, t2[0], t2[1]);
            p3 = addVertex(v1.x, v1.y, v1.z, t1[0], t1[1]);
            baseTriangle.push_back(p1);
            baseTriangle.push_back(p2);
            baseTriangle.push_back(p3);

            p1 = addVertex(v2.x, v2.y, v2.z, t2[0], t2[1]);
            p2 = addVertex(v3.x, v3.y, v3.z, t3[0], t3[1]);
            p3 = addVertex(v4.x, v4.y, v4.z, t4[0], t4[1]);
            baseTriangle.push_back(p1);
            baseTriangle.push_back(p2);
            baseTriangle.push_back(p3);

            pos0[0] = x; pos0[1] = -y; pos0[2] = z;
            computeTexcoordByNewPosition(t0, pos0);
            p1 = addVertex(x, -y, z, t0[0], t0[1]);
            p2 = addVertex(v4.x, v4.y, v4.z, t4[0], t4[1]);
            p3 = addVertex(v3.x, v3.y, v3.z, t3[0], t3[1]);
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
            vertices[(indx1 * BUFFER_SIZE) + 6] = 1.0f; vertices[(indx1 * BUFFER_SIZE) + 7] = 0.0f;
            vertices[(indx2 * BUFFER_SIZE) + 6] = 0.0f; vertices[(indx2 * BUFFER_SIZE) + 7] = 0.0f;
            vertices[(indx3 * BUFFER_SIZE) + 6] = 0.0f; vertices[(indx3 * BUFFER_SIZE) + 7] = 1.0f;
            vertices[(indx4 * BUFFER_SIZE) + 6] = 1.0f; vertices[(indx4 * BUFFER_SIZE) + 7] = 1.0f;
            subDivisionRectangle(lvl, indx1, indx2, indx3, indx4, face);
        }
        setbuffer();
    }

    void loadMaterials() {
        materials.useMaterial = true;
        //std::string materialPath = "res/textures/materials/windswept-wasteland-bl/";
        //std::string albedo = materialPath + "windswept-wasteland_albedo.png";
        //std::string normal = materialPath + "windswept-wasteland_normal-ogl.png";
        //std::string roughness = materialPath + "windswept-wasteland_roughness.png";
        //std::string depth = materialPath + "windswept-wasteland_height.png";
        //std::string metallic = materialPath + "windswept-wasteland_metallic.png";

        //std::string materialPath = "res/textures/materials/mud/";
        //std::string albedo = materialPath + "sphere_DefaultMaterial_Albedo.png";
        //std::string normal = materialPath + "sphere_DefaultMaterial_Normal.png";
        //std::string roughness = materialPath + "sphere_DefaultMaterial_Roughness.png";

        std::string materialPath = "res/textures/materials/concrete_hexagon/";
        std::string albedo = materialPath + "albedo.jpg";
        std::string normal = materialPath + "normal.png";
        std::string roughness = materialPath + "roughness.jpg";
        std::string metallic = materialPath + "metallic.jpg";
        std::string occlusion = materialPath + "ao.jpg";
        std::string depth = materialPath + "height.png";

        materials.albedoMap = materials.loadTexture(albedo);
        materials.normalMap = materials.loadTexture(normal);
        materials.roughnessMap = materials.loadTexture(roughness);
        materials.depthMap = materials.loadTexture(depth);
        materials.occlusionMap = materials.loadTexture(occlusion);
        materials.metallicMap = materials.loadTexture(metallic);
    }

    void drawNormalLine(Shader* shader, const glm::mat4& projection, const glm::mat4& view) {

        shader->use();

        shader->setMat4("model", model);
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);

        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glDrawElements(GL_POINTS, indices.size(), GL_UNSIGNED_INT, (void*)0);

        glBindVertexArray(0);

    }

    void draw(Shader* shader, const glm::mat4& projection, const glm::mat4& view, glm::vec3& cameraPos, 
              const float &_time, std::map<std::string, unsigned int>& mappers, std::vector<glm::vec3> lightPos, GUI::PBRParam& pbr) {
        shader->use();

        //glm::mat4 m = glm::rotate(model, _time, glm::vec3(0.0f, 1.0f, 0.0f));

        shader->setMat4("model", model);
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);

        for (int i = 0; i < lightPos.size(); i++) {
            shader->setVec3("lightPosition[" + std::to_string(i) + "]", lightPos[i]);
        }
        //shader->setVec3("lightPos", lightPos);
        shader->setVec3("viewPos", cameraPos);

        shader->setVec4("baseColor", pbr.m_BaseColor);
        shader->setFloat("roughnessFactor", pbr.m_RoughnessFactor);
        shader->setFloat("subSurface", pbr.m_SubSurface);
        shader->setFloat("metallicFactor", pbr.m_MetallicFactor);

        shader->setFloat("_Specular", pbr.m_Specular);
        shader->setFloat("_SpecularTint", pbr.m_SpecularTint);
        shader->setFloat("_Sheen", pbr.m_Sheen);
        shader->setFloat("_SheenTint", pbr.m_SheenTint);
        shader->setFloat("_Anisotropic", pbr.m_Anisotropic);
        shader->setFloat("_ClearCoatGloss", pbr.m_ClearCoatGloss);
        shader->setFloat("_ClearCoat",pbr.m_ClearCoat);

        shader->setFloat("heightScale", pbr.m_HeightScale);

        glBindVertexArray(vao);

        if (materials.useMaterial) {
            shader->setBool("useAlbedoMapping", true);
            shader->setInt("albedoMap", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, materials.albedoMap);

            if (materials.normalMap) {
                shader->setBool("useNormalMapping", true);
                shader->setInt("normalMap", 1);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, materials.normalMap);
            }

            if (materials.roughnessMap) {
                shader->setBool("useRoughnessMapping", true);
                shader->setInt("roughnessMap", 2);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, materials.roughnessMap);
            }

            if (materials.metallicMap) {
                shader->setBool("useMetallicMapping", true);
                shader->setInt("metallicMap", 3);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, materials.metallicMap);
            }

            if (materials.occlusionMap) {
                shader->setBool("useOcclusionMapping", true);
                shader->setInt("occlusionMap", 4);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, materials.occlusionMap);
            }
            if (materials.depthMap) {
                shader->setBool("useDepthMapping", true);
                shader->setInt("depthMap", 5);
                glActiveTexture(GL_TEXTURE5);
                glBindTexture(GL_TEXTURE_2D, materials.depthMap);
            }
        }

        shader->setInt("irradianceMap", 6);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mappers["irradianceMap"]);

        shader->setInt("preFilterMap", 7);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mappers["preFilterMap"]);

        shader->setInt("brdfLUTTexture", 8);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, mappers["brdfLUTTexture"]);

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
        //tangent
        vertices.push_back(x * lengthInv);
        vertices.push_back(y * lengthInv);
        vertices.push_back(z * lengthInv);
        //bitangent 
        vertices.push_back(x * lengthInv);
        vertices.push_back(y * lengthInv);
        vertices.push_back(z * lengthInv);
        countVertex++;

        return countVertex - 1;
    }

    void fixingUVTriangle(unsigned int indx1, unsigned int indx2, unsigned int indx3) {
        //fix UV
        float t1[2] = { vertices[indx1 * BUFFER_SIZE + 6], vertices[indx1 * BUFFER_SIZE + 7] };
        float t2[2] = { vertices[indx2 * BUFFER_SIZE + 6], vertices[indx2 * BUFFER_SIZE + 7] };
        float t3[2] = { vertices[indx3 * BUFFER_SIZE + 6], vertices[indx3 * BUFFER_SIZE + 7] };

        float p1[3] = { vertices[indx1 * BUFFER_SIZE], vertices[indx1 * BUFFER_SIZE + 1], vertices[indx1 * BUFFER_SIZE + 2] };
        float p2[3] = { vertices[indx2 * BUFFER_SIZE], vertices[indx2 * BUFFER_SIZE + 1], vertices[indx2 * BUFFER_SIZE + 2] };
        float p3[3] = { vertices[indx3 * BUFFER_SIZE], vertices[indx3 * BUFFER_SIZE + 1], vertices[indx3 * BUFFER_SIZE + 2] };

        if (t2[0] - t1[0] >= 0.5f && t1[1] != 1.0f) t2[0] -= 1.0f;
        if (t3[0] - t2[0] > 0.5f) t3[0] -= 1.0f;
        if (t1[0] > 0.5f && t1[0] - t3[0] > 0.5f || t1[0] == 1.0f && t3[1] == 0.0f) t1[0] -= 1.0f;
        if (t2[0] > 0.5f && t2[0] - t1[0] > 0.5f) t2[0] -= 1.0f;
        if (t1[1] == 0.0f || t1[1] == 1.0f) t1[0] = (t2[0] + t3[0]) / 2.0f;
        if (t2[1] == 0.0f || t2[1] == 1.0f) t2[0] = (t1[0] + t3[0]) / 2.0f;
        if (t3[1] == 0.0f || t3[1] == 1.0f) t3[0] = (t1[0] + t2[0]) / 2.0f;

        if (t2[0] - t1[0] < 0.5f && t3[0] - t2[0] < 0.5f && t3[0] - t1[0] > 0.5f) {
            if (t2[1] == 1.0f || t2[1] == 0.0f) {
                t3[0] -= 1.0f;
                t2[0] = (t1[0] + t3[0]) / 2.0f;
            }
        }

        vertices[indx1 * BUFFER_SIZE + 6] = t1[0]; vertices[indx1 * BUFFER_SIZE + 7] = t1[1];
        vertices[indx2 * BUFFER_SIZE + 6] = t2[0]; vertices[indx2 * BUFFER_SIZE + 7] = t2[1];
        vertices[indx3 * BUFFER_SIZE + 6] = t3[0]; vertices[indx3 * BUFFER_SIZE + 7] = t3[1];
    }

    void setTangentAndBitangent(unsigned int indx1, unsigned int indx2, unsigned int indx3) {

        glm::vec2 t1(vertices[indx1 * BUFFER_SIZE + 6], vertices[indx1 * BUFFER_SIZE + 7]);
        glm::vec2 t2(vertices[indx2 * BUFFER_SIZE + 6], vertices[indx2 * BUFFER_SIZE + 7]);
        glm::vec2 t3(vertices[indx3 * BUFFER_SIZE + 6], vertices[indx3 * BUFFER_SIZE + 7]);

        glm::vec3 pos1(vertices[indx1 * BUFFER_SIZE], vertices[indx1 * BUFFER_SIZE + 1], vertices[indx1 * BUFFER_SIZE + 2]);
        glm::vec3 pos2(vertices[indx2 * BUFFER_SIZE], vertices[indx2 * BUFFER_SIZE + 1], vertices[indx2 * BUFFER_SIZE + 2]);
        glm::vec3 pos3(vertices[indx3 * BUFFER_SIZE], vertices[indx3 * BUFFER_SIZE + 1], vertices[indx3 * BUFFER_SIZE + 2]);

        std::pair<glm::vec3,glm::vec3> tb = materials.calculateTangentSpace(pos1, pos2, pos3, t1, t2, t3);

        unsigned int stTangent = 8;
        unsigned int stBitangent = 11;
        for (int i = 0; i < 3; i++) {
            vertices[indx1 * BUFFER_SIZE + stTangent + i] = tb.first[i];
            vertices[indx2 * BUFFER_SIZE + stTangent + i] = tb.first[i];
            vertices[indx3 * BUFFER_SIZE + stTangent + i] = tb.first[i];

            vertices[indx1 * BUFFER_SIZE + stBitangent + i] = tb.second[i];
            vertices[indx2 * BUFFER_SIZE + stBitangent + i] = tb.second[i];
            vertices[indx3 * BUFFER_SIZE + stBitangent + i] = tb.second[i];
        }
    }

    unsigned int cloneVertex(unsigned int indx) {
        unsigned int clone = 0;

        float pos[3] = { vertices[indx * BUFFER_SIZE], vertices[indx * BUFFER_SIZE + 1], vertices[indx * BUFFER_SIZE + 2] };
        float tex[2] = { vertices[indx * BUFFER_SIZE + 6], vertices[indx * BUFFER_SIZE + 7] };
        clone = addVertex(pos[0], pos[1], pos[2], tex[0], tex[1]);

        return clone;
    }

    void subDivisionTriangle(int lvl, unsigned int indx1, unsigned int indx2, unsigned int indx3) {
        sortVertex(indx1, indx2, indx3);
        if (lvl == 0) {
            //printf("find %d %d %d ---- ", indx1, indx2, indx3);
            //if (duplicatedVertex[indx1] == true) indx1 = cloneVertex(indx1);
            //if (duplicatedVertex[indx2] == true) indx2 = cloneVertex(indx2);
            //if (duplicatedVertex[indx3] == true) indx3 = cloneVertex(indx3);
            //printf("%d %d %d\n", indx1, indx2, indx3);
            fixingUVTriangle(indx1, indx2, indx3);
            setTangentAndBitangent(indx1, indx2, indx3);
            
            indices.push_back(indx1); duplicatedVertex[indx1] = true;
            indices.push_back(indx2); duplicatedVertex[indx2] = true;
            indices.push_back(indx3); duplicatedVertex[indx3] = true;

            return;
        }
        int p1 = indx1 * BUFFER_SIZE;
        int p2 = indx2 * BUFFER_SIZE;
        int p3 = indx3 * BUFFER_SIZE;

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

        computeTexcoordByNewPosition(newt1, newV1);
        computeTexcoordByNewPosition(newt2, newV2);
        computeTexcoordByNewPosition(newt3, newV3);

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
        int p1 = indx1 * BUFFER_SIZE;
        int p2 = indx2 * BUFFER_SIZE;
        int p3 = indx3 * BUFFER_SIZE;
        int p4 = indx4 * BUFFER_SIZE;

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
        int idx = indx * BUFFER_SIZE;
        int ret = addVertex(vertices[idx], vertices[idx + 1], vertices[idx + 2]);
        faces.push_back(face);
        return ret;
    }

    void computeHalfTexcoord(const float t1[2], const float t2[2], float newTex[2]) {
        newTex[0] = (t1[0] + t2[0]) / 2.0f;
        newTex[1] = (t1[1] + t2[1]) / 2.0f;
    }

    void computeTexcoordByNewPosition(float newTex[2], const float pos[3]) {
        float length = glm::sqrt((pos[0] * pos[0]) + (pos[1] * pos[1]) + (pos[2] * pos[2]));
        glm::vec3 n = { pos[0] / length, pos[1] / length, pos[2] / length };

        newTex[0] = (std::atan2(n.x, n.z) / (2.0f * PI)) + 0.5f;
        newTex[1] = (std::asin(-n.y) / PI) + 0.5f;
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

    void sortVertex(unsigned int& p1, unsigned int& p2, unsigned int& p3) {

        typedef std::pair<std::pair<float, float>, unsigned int> coordMap;
        coordMap v1 = { {vertices[p1 * BUFFER_SIZE + 6], vertices[p1 * BUFFER_SIZE + 7]}, p1 };
        coordMap v2 = { {vertices[p2 * BUFFER_SIZE + 6], vertices[p2 * BUFFER_SIZE + 7]}, p2 };
        coordMap v3 = { {vertices[p3 * BUFFER_SIZE + 6], vertices[p3 * BUFFER_SIZE + 7]}, p3 };

        std::vector<coordMap> sortingVertex;
        sortingVertex.push_back(v1);
        sortingVertex.push_back(v2);
        sortingVertex.push_back(v3);

        std::sort(sortingVertex.begin(), sortingVertex.end(), [](coordMap a, coordMap b) {
            if (a.first.first < b.first.first) return true;
            if (a.first.first == b.first.first) {
                if (a.first.second < b.first.second) return true;
                if (a.first.second == b.first.second){
                    if (a.second < b.second) return true;
                    else return false;
                }
            }
            return false;
            });

        p1 = sortingVertex[0].second;
        p2 = sortingVertex[1].second;
        p3 = sortingVertex[2].second;
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
        int VSIZE = BUFFER_SIZE;
        unsigned int sizeBuffer = vertices.size() * sizeof(float);
        glBufferData(GL_ARRAY_BUFFER, sizeBuffer, &vertices.at(0), GL_STATIC_DRAW);
        //pos
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * VSIZE, (void*)0);
        glEnableVertexAttribArray(0);
        //normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * VSIZE, (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(1);
        //tex coord
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * VSIZE, (void*)(sizeof(float) * 6));
        glEnableVertexAttribArray(2);
        //tangent
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float) * VSIZE, (void*)(sizeof(float) * 8));
        glEnableVertexAttribArray(3);
        //bitangent
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(float) * VSIZE, (void*)(sizeof(float) * 11));
        glEnableVertexAttribArray(4);

        if (faces.size() != 0) {
            unsigned int vboFace;
            glGenBuffers(1, &vboFace);
            glBindBuffer(GL_ARRAY_BUFFER, vboFace);
            glBufferData(GL_ARRAY_BUFFER, faces.size() * sizeof(unsigned int), &faces.at(0), GL_STATIC_DRAW);
        }

        printf("face size: %d\n", faces.size());

        glBindVertexArray(0);
    }

    void getTexturePath() {
        texturePaths.push_back("res/textures/earth2048.bmp");
        texturePaths.push_back("res/textures/moon1024.bmp");
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