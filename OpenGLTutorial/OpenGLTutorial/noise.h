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

    static std::vector<std::vector<float>> GenerateNoiseMap(int mapWidth, int mapHeight, int seed, float scale, 
        int octaves, float persistence, float lacunarity, glm::vec2 offset) {
	    if (scale <= 0) {
		    scale = 0.000001f;
	    }
        std::vector<std::vector<float>> noiseMap(mapHeight, std::vector<float>(mapWidth, 0));
        Perlin perlin(0);

        float maxNoiseHeight = MIN_VALUE;
        float minNoiseHeight = MAX_VALUE;

        float halfWidth = mapWidth / 2.0f;
        float halfHeight = mapHeight/ 2.0f;

        for (int y = 0; y < mapHeight; y++) {
            for (int x = 0; x < mapWidth; x++) {
                float sampleX = (x - halfWidth) / scale;
                float sampleY = (y - halfHeight) / scale;

                float perlinValue = (float)perlin.OctavePerlin(sampleX, sampleY, 0, seed,
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
                noiseMap[y][x] = perlin.inverseLerp(minNoiseHeight, maxNoiseHeight, noiseMap[y][x]);
            }
        }
        return noiseMap;
	}
};


#endif
