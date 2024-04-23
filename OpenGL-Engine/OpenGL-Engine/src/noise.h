#pragma once
#ifndef NOISE_H
#define NOISE_H

#include <glm/glm.hpp>
#include <vector>

#include "Perlin.h"

const float MAX_VALUE = 3.40282347E+38F;
const float MIN_VALUE = -3.40282347E+38F;

class Noise {
public:

    enum NormalizeMode { Local, Global };

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
};

#endif
