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

	std::vector<std::vector<float>> GenerateNoiseMap(int mapWidth, int mapHeight, float scale, int octaves, float persistence, float lacunarity) {
		if (scale <= 0) {
			scale = 0.000001f;
		}
		std::vector<std::vector<float>> noiseMap(mapHeight, std::vector<float>(mapWidth, 0));
        Perlin perlin(0);

        float maxNoiseHeight = MIN_VALUE;
        float minNoiseHeight = MAX_VALUE;

        for (int y = 0; y < mapHeight; y++) {
            for (int x = 0; x < mapWidth; x++) {
                float sampleX = x / scale;
                float sampleY = y / scale;

                float perlinValue = (float)perlin.OctavePerlin(sampleX, sampleY, 0,
                    octaves, persistence, lacunarity);

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