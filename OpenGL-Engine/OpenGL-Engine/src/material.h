#pragma once
#ifndef MATERIALS_H
#define MATERIALS_H

#include <glm/glm.hpp>
#include <JoeyDeVries/filesystem.h>
#include <vector>
#include <math.h>
#include <algorithm>
#include <string>

#include "shader_m.h"
#include <TinyGLTF/stb_image.h>

class Materials {
public:
    
    bool useMaterial;

    glm::vec4 baseColor;
    unsigned int albedoMap, normalMap, roughnessMap, emissiveMap, metallicMap, occlusionMap, depthMap;
    unsigned int metallicRoughnessOcclusionTexture;
    float metallicFactor, roughnessFactor;
    std::map<std::string, unsigned int> Map;

    int width, height;

    Materials() {

        useMaterial = false;
        baseColor = glm::vec4(1.0f);

        albedoMap = 0;
        normalMap = 0;
        roughnessMap = 0;
        emissiveMap = 0;
        metallicMap = 0;
        occlusionMap = 0;
        depthMap = 0;
        metallicRoughnessOcclusionTexture = 0;

        metallicFactor = 0.0f;
        roughnessFactor = 0.0f;

        width = 0;
        height = 0;

        Map.clear();
    }

    ~Materials() {

    }

    unsigned int loadTexture(std::string& path) {
        unsigned int tex = 0;
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
        width = (float)w;
        height = (float)h;
        stbi_image_free(data);

        printf("load materials = %s\nresult = %d\n", path.c_str(), tex);
        return tex;
    }

    std::pair<glm::vec3,glm::vec3> calculateTangentSpace(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 pos3, 
                                 glm::vec2 uv1,  glm::vec2 uv2,  glm::vec2 uv3 )
    {
        glm::mat3 btn(1.0f);

        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent(0.0f);
        glm::vec3 bitangent(0.0f);

        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent = glm::normalize(tangent);

        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent = glm::normalize(bitangent);

        return {tangent, bitangent};
    }
};

#endif
