#pragma once
#ifndef PLANE_H
#define PLANE_H

#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>
#include <queue>

#include "shader_m.h"
#include "noise.h"

class Terrain {
public:
	std::string name;
	glm::vec3 color;
	float height;

	void setRegion(const std::string& S, const glm::vec3& C, const float& H) {
		name = S;
		color = C;
		height = H;
	}
};

class TerrainChunk {
public:

	unsigned int vao, tex;
	glm::vec3 pos;
	bool visible;

	TerrainChunk() {
	}
	~TerrainChunk() {
	}

	void setPos(const glm::vec3& p) {
		pos = p;
	}
};

class Plane {

public:

	enum DrawMode { NoiseMap, ColorMap };
	DrawMode drawMode;

	int planeSize;
	int chunkSize;
	float fov;

	unsigned int ebo, vao;
	unsigned int noiseTex;

	unsigned int indicesCount;
	unsigned int componentType;

	std::vector<Terrain> terrains;

	std::vector<TerrainChunk> terrainChunks;
	std::map<std::pair<float, float>, int> DictTerrainChunk;
	std::queue<int> queueDraw;

	Plane(const int& planesize) {

		planeSize = planesize;
		printf("plane %d\n", planeSize);
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		generatePlane(planeSize);
		generateColorTerrain();
	
		glBindVertexArray(0);
	}

	std::pair<GLuint,GLuint> GenerateNoiseMap(int width, int height, int seed, float scale, int octaves, float persistence, float lacunarity, glm::vec2 offset) {

		Noise noise;
		std::vector<std::vector<float>> noiseMap = noise.GenerateNoiseMap(width + 1, height + 1, seed, scale, octaves, persistence, lacunarity, offset, noise.Global);

		//printf("generate Noise map finished\n");
		//printf("noiseMap size y = %d\n", noiseMap.size());
		//printf("noiseMap size x = %d\n", noiseMap[0].size());
		std::vector<glm::vec4> aColor(height * width);
		std::vector<float> aHeight;
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				for (int k = 0; k < terrains.size(); k++) {
					if (noiseMap[y][x] >= terrains[k].height) {
						aColor[y * height + x] = glm::vec4(terrains[k].color, 1.0f);
					} else {
						break;
					}
				}
				// noiseMap;
				//color.push_back(glm::vec4(noiseMap[y][x], noiseMap[y][x], noiseMap[y][x], 1.0f));
			}
		}

		int n = noiseMap.size();
		for (int y = 0; y < n - 1; y++) {
			int m = noiseMap[y].size();
			aHeight.push_back(noiseMap[y][0]);
			for (int x = 0; x < m; x++) {
				aHeight.push_back(noiseMap[y][x]);
				aHeight.push_back(noiseMap[y + 1 < n ? y + 1 : n - 1][x]);
			}
			aHeight.push_back(noiseMap[y + 1][m - 1]);
		}
		//printf("acolor size = %d\n", aColor.size());
		std::pair<GLuint, GLuint> ret;
		glGenVertexArrays(1, &ret.first);
		glBindVertexArray(ret.first);

		unsigned int hVbo;
		int HeightSize = aHeight.size() * sizeof(float);
		glGenBuffers(1, &hVbo);
		glBindBuffer(GL_ARRAY_BUFFER, hVbo);
		glBufferData(GL_ARRAY_BUFFER, HeightSize, &aHeight.at(0), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glGenTextures(1, &ret.second);
		glBindTexture(GL_TEXTURE_2D, ret.second);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, &aColor.at(0));

		glBindVertexArray(0);

		return ret;
	}

	void InitTerrainChunk(const int &chunksize, const float &visibleDistance, const glm::vec3 &cameraPos) {
		chunkSize = chunksize - 1;
		fov = visibleDistance;
	}

	void update(const glm::vec3 &cameraPos) {
		/*for (TerrainChunk& tc : terrainChunks) {
			tc.visible = false;
		}*/
		std::pair<float, float> pos;
		for (int i = -chunkSize; i <= chunkSize; i++) {
			for (int j = -chunkSize; j <= chunkSize; j++) {
				TerrainChunk tc;

				pos.first = ((int)cameraPos.x - ((int)cameraPos.x % (int)planeSize));
				pos.second = ((int)cameraPos.z - ((int)cameraPos.z % (int)planeSize));

				pos.first += (j * planeSize);
				pos.second += (i * planeSize);

				tc.setPos(glm::vec3(pos.first, 0.0f, pos.second));
				tc.visible = true;
				if (DictTerrainChunk.find(pos) == DictTerrainChunk.end()) {
					std::pair<GLuint, GLuint> res = GenerateNoiseMap(planeSize, planeSize, 4, 27.9f, 4, 0.5f, 2.0f, glm::vec2(pos.first, pos.second));
					tc.vao = res.first;
					tc.tex = res.second;
					terrainChunks.push_back(tc);
					DictTerrainChunk.insert({ pos, int(terrainChunks.size() - 1) });
				}
				int indx = DictTerrainChunk[pos];
				terrainChunks[indx].visible = true;
				queueDraw.push(indx);
			}
		}
	}

	void draw(Shader& shader, const glm::mat4& projection, const glm::mat4& view, const int &np, const glm::vec3 &cameraPos) {

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		shader.use();
		shader.setInt("noiseMap", 0);

		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.setFloat("lenght", np + 1);
		shader.setFloat("heightMultiplier", 20.0f);

		while (!queueDraw.empty()) {
			int indx = queueDraw.front();
			queueDraw.pop();
			TerrainChunk &tc = terrainChunks[indx];
			if (tc.visible) {
				glBindVertexArray(tc.vao);
				glActiveTexture(GL_TEXTURE0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

				glm::mat4 model(1.0f);
				model = glm::translate(model, tc.pos);
				shader.setMat4("model", model);
				glBindTexture(GL_TEXTURE_2D, tc.tex);
				glDrawElements(GL_TRIANGLE_STRIP, indicesCount, componentType, (void*)(0));
			}
			glBindVertexArray(0);
		}

		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(0);
	}

	~Plane() {

	}

private:

	void generatePlane(const int &n) {
		
		printf("masuk ke plane indices\n");
		std::vector<unsigned int> indices;

		unsigned int indx = 0;
		for (int row = 0; row < n; row++) {
			// additional triangle on left
			indices.push_back(indx++);
			for (int col = 0; col < n + 1; col++) {
				// below
				indices.push_back(indx++);
				// upper
				indices.push_back(indx++);
			}
			// additional triangle on right
			indices.push_back(indx++);
		}

		glBindVertexArray(vao);

		indicesCount = indices.size() * sizeof(unsigned int);
		componentType = GL_UNSIGNED_INT;

		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount, &indices.at(0), GL_STATIC_DRAW);

		printf("size indices: %d\n", indices.size());
	}

	void generateColorTerrain() {

		Terrain region;
		region.setRegion("deep water", { 0.058, 0.368, 0.611 }, 0.0f);
		terrains.push_back(region);
		region.setRegion("water", { 0.109, 0.639, 0.925 }, 0.4);
		terrains.push_back(region);
		region.setRegion("sand", { 0.941 ,0.929, 0.643 }, 0.45);
		terrains.push_back(region);
		region.setRegion("grass 1", { 0.325, 0.82, 0.204 }, 0.55);
		terrains.push_back(region);
		region.setRegion("grass 2", { 0.066, 0.486, 0.074 }, 0.65);
		terrains.push_back(region);
		region.setRegion("rock 1", { 0.333, 0.254, 0.141 }, 0.8);
		terrains.push_back(region);
		region.setRegion("rock 2", { 0.235, 0.145, 0.082 }, 0.9);
		terrains.push_back(region);
		region.setRegion("snow top", { 1.000, 1.000, 1.000 }, 1.00);
		terrains.push_back(region);
	}
};

#endif
