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
	int border;
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
		border = planeSize + 2;
		printf("plane %d\n", planeSize);
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		generatePlane(planeSize);
		generateColorTerrain();
	
		glBindVertexArray(0);
	}

	std::pair<GLuint,GLuint> GenerateNoiseMap(int width, int height, int seed, float scale, int octaves, float persistence, float lacunarity, glm::vec2 offset, const float &heightMultiplier) {

		Noise noise;
		std::vector<std::vector<float>> noiseMap = noise.GenerateNoiseMap(width + 3, height + 3, seed, scale, octaves, persistence, lacunarity, offset - 1.0f, noise.Global);

		int n = noiseMap.size();
		int m = noiseMap[0].size();
		//printf("generate Noise map finished\n");
		//printf("noiseMap size y = %d\n", noiseMap.size());
		//printf("noiseMap size x = %d\n", noiseMap[0].size());
		std::vector<glm::vec4> aColor;
		for (int y = 1; y < n - 2; y++) {
			for (int x = 1; x < m - 2; x++) {
				int flag = 0;
				for (int k = 0; k < terrains.size(); k++) {
					if (noiseMap[y][x] >= terrains[k].height) {
						flag = k;
					} else {
						break;
					}
				}
				aColor.push_back(glm::vec4(terrains[flag].color, 1.0f));
				// noiseMap;
				//aColor.push_back(glm::vec4(noiseMap[y][x], noiseMap[y][x], noiseMap[y][x], 1.0f));
			}
		}
		std::vector<float> heightMap;
		std::vector<glm::ivec2> indxMap;
		for (int y = 0; y < n - 1; y++) {
			for (int x = 0; x < m; x++) {
				heightMap.push_back(calculateHeightValue(noiseMap[y][x], heightMultiplier));
				indxMap.push_back({x,y});

				heightMap.push_back(calculateHeightValue(noiseMap[y + 1][x], heightMultiplier));
				indxMap.push_back({ x,y + 1 });
			}
		}
		int offsetVertices = n * 2;
		float halfOffset = (float)n / 2.0f;
		std::vector<glm::vec3> normalMap(heightMap.size(), glm::vec3(0.0f));
		for (int i = 0; i < (int)heightMap.size() - 3; i++) {

			glm::vec3 pos0((float)indxMap[i].x - halfOffset, heightMap[i], (float)indxMap[i].y - halfOffset);
			glm::vec3 pos1((float)indxMap[i + 1].x - halfOffset, heightMap[i + 1], (float)indxMap[i + 1].y - halfOffset);
			glm::vec3 pos2((float)indxMap[i + 2].x - halfOffset, heightMap[i + 2], (float)indxMap[i + 2].y - halfOffset);

			glm::vec3 vec01 = pos1 - pos0;
			glm::vec3 vec02 = pos2 - pos0;
			if (i % 2 == 1) {
				vec01 = pos2 - pos0;
				vec02 = pos1 - pos0;
			}
			glm::vec3 norm = glm::cross(vec01, vec02);
			norm = glm::normalize(norm);

			normalMap[i] += norm;
			normalMap[i + 1] += norm;
			normalMap[i + 2] += norm;
		}

		for (int i = 0; i < (int)normalMap.size(); i++) {
			int rowIndex = i % offsetVertices;
			int colIndex = i / offsetVertices;
			if (i % 2 == 1 && colIndex < n - 2) {
				int indx = (colIndex + 1) * offsetVertices + rowIndex - 1;
				normalMap[i] += normalMap[indx];
			}
			if (i % 2 == 0 && colIndex > 0) {
				int indx = (colIndex - 1) * offsetVertices + rowIndex + 1;
				normalMap[i] = normalMap[indx];
			}
			normalMap[i] = glm::normalize(normalMap[i]);
		}

		std::vector<float> aHeight;
		std::vector<glm::vec3> aNormal;
		for (int i = 0; i < (int)normalMap.size();i++) {
			glm::ivec2 &coord = indxMap[i];
			if (coord.x > 0 && coord.y && coord.x <= width + 1 && coord.y <= height + 1) {
				int colIndex = i / offsetVertices;
				if (colIndex == 0 || colIndex == n - 1) continue;
				if (coord.x == 1 && i % 2 == 0) {
					aHeight.push_back(heightMap[i]);
					aNormal.push_back(normalMap[i]);
				}
				aHeight.push_back(heightMap[i]);
				aNormal.push_back(normalMap[i]);
				if (coord.x == m - 2 && i % 2 == 1) {
					aHeight.push_back(heightMap[i]);
					aNormal.push_back(normalMap[i]);
				}
			}
		}

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

		unsigned int normVbo;
		int normSize = aNormal.size() * sizeof(float) * 3;
		glGenBuffers(1, &normVbo);
		glBindBuffer(GL_ARRAY_BUFFER, normVbo);
		glBufferData(GL_ARRAY_BUFFER, normSize, &aNormal.at(0), GL_STATIC_DRAW);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
		glEnableVertexAttribArray(1);

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

	void update(const glm::vec3 &cameraPos, const float &heightMultiplier) {
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
					std::pair<GLuint, GLuint> res = GenerateNoiseMap(planeSize, planeSize, 4, 27.9f, 4, 0.5f, 2.0f, glm::vec2(pos.first, pos.second), heightMultiplier);
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

		shader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
		shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		shader.setVec3("lightPos", glm::vec3(1.0f, 0.0f, 0.0f));
		shader.setVec3("viewPos", cameraPos);

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
	
	static float calculateHeightValue(const float& v, const float& heightMultiplier) {
		return (v * heightMultiplier) - heightMultiplier;
	}

};

#endif
