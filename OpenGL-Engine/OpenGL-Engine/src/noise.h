#pragma once
#ifndef NOISE_H
#define NOISE_H

#include <glm/glm.hpp>
#include <vector>

#include "Perlin.h"
#include "computeShader.h"

const float MAX_VALUE = 3.40282347E+38F;
const float MIN_VALUE = -3.40282347E+38F;

class Noise {
public:

    enum NormalizeMode { Local, Global };
    
    void generateNoiseMap_Compute(unsigned int &noiseTex, int w, int h) {

        glGenTextures(1, &noiseTex);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, noiseTex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
        glBindImageTexture(0, noiseTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

        //coutLimitation();
    }

    static std::vector<std::vector<float>> GenerateNoiseMap(int mapWidth, int mapHeight, int seed, float scale, 
        int octaves, float persistence, float lacunarity, glm::vec2 offset, NormalizeMode normalizeMode) {
	    if (scale <= 0) {
		    scale = 0.000001f;
	    }
        std::vector<std::vector<float>> noiseMap(mapHeight, std::vector<float>(mapWidth, 0));
        Perlin perlin(0);

        float maxNoiseHeight = MIN_VALUE;
        float minNoiseHeight = MAX_VALUE;

        float halfWidth = mapWidth / 2.0f;
        float halfHeight = mapHeight / 2.0f;

        float maxPossibleHeight = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;

        for (int i = 0; i < octaves; i++) {
            maxPossibleHeight += amplitude;
            amplitude *= persistence;
        }

        for (int y = 0; y < mapHeight; y++) {
            for (int x = 0; x < mapWidth; x++) {
                float sampleX = (x - halfWidth);
                float sampleY = (y - halfHeight);

                float perlinValue = (float)perlin.OctavePerlin(sampleX, sampleY, 0, scale, seed,
                    octaves, persistence, lacunarity, offset);

                if (maxNoiseHeight < perlinValue) {
                    maxNoiseHeight = perlinValue;
                }

                if (minNoiseHeight > perlinValue) {
                    minNoiseHeight = perlinValue;
                }
                noiseMap[y][x] = perlinValue;
            }
        }

        for (int y = 0; y < mapHeight; y++) {
            for (int x = 0; x < mapWidth; x++) {
                if (normalizeMode == Local) {
                    noiseMap[y][x] = interpolate::inverseLerp(minNoiseHeight, maxNoiseHeight, noiseMap[y][x]);
                } else {
                    float normalizedHeight = (noiseMap[y][x] + 1) / (2.0f * maxPossibleHeight / 2.75f);
                    noiseMap[y][x] = clamp(normalizedHeight, 0.0f, MAX_VALUE);
                }
            }
        }
        return noiseMap;
	}
private:

    static float clamp(const float &v, const float &lower, const float &upper) {
        return std::max(lower, std::min(v, upper));
    }

    void coutLimitation() {
        // query limitations
        // -----------------
        int max_compute_work_group_count[3];
        int max_compute_work_group_size[3];
        int max_compute_work_group_invocations;

        for (int idx = 0; idx < 3; idx++) {
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, idx, &max_compute_work_group_count[idx]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, idx, &max_compute_work_group_size[idx]);
        }
        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_compute_work_group_invocations);

        std::cout << "OpenGL Limitations: " << std::endl;
        std::cout << "maximum number of work groups in X dimension " << max_compute_work_group_count[0] << std::endl;
        std::cout << "maximum number of work groups in Y dimension " << max_compute_work_group_count[1] << std::endl;
        std::cout << "maximum number of work groups in Z dimension " << max_compute_work_group_count[2] << std::endl;

        std::cout << "maximum size of a work group in X dimension " << max_compute_work_group_size[0] << std::endl;
        std::cout << "maximum size of a work group in Y dimension " << max_compute_work_group_size[1] << std::endl;
        std::cout << "maximum size of a work group in Z dimension " << max_compute_work_group_size[2] << std::endl;

        std::cout << "Number of invocations in a single local work group that may be dispatched to a compute shader " << max_compute_work_group_invocations << std::endl;
    }
};

#endif
