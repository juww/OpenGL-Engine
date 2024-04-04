#pragma once
#ifndef PLANE_H
#define PLANE_H

#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>
#include <queue>

#include "shader_m.h"
#include "noise.h"
#include "grass.h"

class Terrain {
public:
	std::string name;
	unsigned int tex;
	glm::vec3 color;
	float colorStrength;
	float height;
	float blend;
	std::string texturePath;

	void setRegion(const std::string& S, const glm::vec3& C, const float& CS, const float& H, const float& B, const std::string& TP) {
		name = S;
		color = C;
		colorStrength = CS;
		height = H;
		blend = B;
		texturePath = TP;
	}
};

class TerrainChunk {
public:

	unsigned int vao;
	glm::vec3 pos;
	bool visible;
	

	TerrainChunk() {
	}
	~TerrainChunk() {
	}
};

class Plane {

public:

	enum DrawMode { NoiseMap, ColorMap };
	DrawMode drawMode;

	int planeSize;
	int chunkSize;
	float fov;

	float minHeight, maxHeight;
	unsigned int spriteTextures;
	unsigned int ebo, vao;
	unsigned int noiseTex;

	unsigned int indicesCount;
	unsigned int componentType;

	std::vector<Terrain> terrains;

	std::vector<TerrainChunk> terrainChunks;
	std::map<std::pair<float, float>, int> DictTerrainChunk;
	std::queue<int> queueDraw;
	Grass grass;

	Plane(const int& planesize) {

		planeSize = planesize;
		printf("plane %d\n", planeSize);
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		minHeight = MAX_VALUE;
		maxHeight = MIN_VALUE;

		generatePlane(planeSize);
		generateColorTerrain();
	
		glBindVertexArray(0);
	}

	TerrainChunk GenerateTerrain(int width, int height, int seed, float scale, int octaves, float persistence, float lacunarity, glm::vec2 offset, const float &heightMultiplier) {

		Noise noise;
		std::vector<std::vector<float>> noiseMap = noise.GenerateNoiseMap(width + 3, height + 3, seed, scale, octaves, persistence, lacunarity, offset - 1.0f, noise.Global);

		int n = noiseMap.size();
		int m = noiseMap[0].size();
		//printf("generate Noise map finished\n");
		//printf("noiseMap size y = %d\n", noiseMap.size());
		//printf("noiseMap size x = %d\n", noiseMap[0].size());
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
		TerrainChunk result;

		glGenVertexArrays(1, &result.vao);
		glBindVertexArray(result.vao);

		int offsetVertices = n * 2;
		float halfOffset = (float)n / 2.0f;
		GenerateHeightMapping(heightMap, indxMap, offsetVertices, n, m, width, height);
		GenerateNormalMapping(heightMap, indxMap, offsetVertices, halfOffset, n, m, width, height);

		glBindVertexArray(0);
		GenerateGrass(heightMap, indxMap, offsetVertices, n, m, width, height);

		return result;
	}

	void initGrass(const int& density) {
		grass.initialize(planeSize, planeSize, density);
	}

	void GenerateGrass(const std::vector<float> &heightMap, const std::vector<glm::ivec2> &indxMap, const int &offsetVertices,
		const int &n, const int &m, const int &width, const int &height) {

		std::vector<glm::vec3> posOffset;
		for (int i = 0; i < (int)heightMap.size() - 3; i++) {
			const glm::ivec2& coord = indxMap[i];
			if (coord.x > 0 && coord.y > 0 && coord.x <= width && coord.y <= height + 1) {
				int colIndex = i / offsetVertices;
				if (colIndex == 0 || colIndex == n - 2) continue;
				const glm::ivec2& c1 = indxMap[i + 1];
				const glm::ivec2& c2 = indxMap[i + 2];
				glm::vec3 pos1(coord.x - 1, heightMap[i], coord.y - 1);
				glm::vec3 pos2(c1.x - 1, heightMap[i + 1], c1.y - 1);
				glm::vec3 pos3(c2.x - 1, heightMap[i + 2], c2.y - 1);

				for (int k = 0; k < grass.density; k++) {

					float t = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
					glm::vec3 l1 = lerp(t, pos1, pos2);
					t = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
					glm::vec3 l2 = lerp(t, pos1, pos3);
					t = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
					glm::vec3 l3 = lerp(t, l1, l2);

					posOffset.push_back(l3);
				}

			}
		}
		grass.setPositionGrass(posOffset);
	}

	void drawGrass(Shader &shader, const glm::mat4& view, const glm::mat4& projection) {
		grass.draw(shader, view, projection);
	}

	void InitTerrainChunk(const int &chunksize, const float &visibleDistance, const glm::vec3 &cameraPos) {
		chunkSize = chunksize - 1;
		fov = visibleDistance;
	}

	void update(const glm::vec3 &cameraPos, int seed, float scale, int octaves, float persistence, float lacunarity, glm::vec2 offset, const float& heightMultiplier, static bool &changeParam) {

		std::pair<float, float> pos;
		for (int i = -chunkSize; i <= chunkSize; i++) {
			for (int j = -chunkSize; j <= chunkSize; j++) {

				pos.first = ((int)cameraPos.x - ((int)cameraPos.x % (int)planeSize));
				pos.second = ((int)cameraPos.z - ((int)cameraPos.z % (int)planeSize));

				pos.first += (j * planeSize);
				pos.second += (i * planeSize);

				if (DictTerrainChunk.find(pos) == DictTerrainChunk.end()) {
					TerrainChunk tc = GenerateTerrain(planeSize, planeSize, seed, scale, octaves, persistence, lacunarity, glm::vec2(pos.first, pos.second) + offset, heightMultiplier);
					tc.pos = { pos.first, 0.0f ,pos.second };
					terrainChunks.push_back(tc);
					DictTerrainChunk.insert({ {pos.first, pos.second}, int(terrainChunks.size() - 1) });
				}
				int indx = DictTerrainChunk[pos];
				if (changeParam) {
					TerrainChunk tc = GenerateTerrain(planeSize, planeSize, seed, scale, octaves, persistence, lacunarity, glm::vec2(pos.first, pos.second) + offset, heightMultiplier);
					tc.pos = { pos.first, 0.0f ,pos.second };
					terrainChunks[indx] = tc;
					changeParam = false;
				}
				terrainChunks[indx].visible = true;
				queueDraw.push(indx);
			}
		}
	}

	void loadSpriteTexture(const int& width, const int& height, const int& layerCount, const int& mipLevelCount) {
		glGenTextures(1, &spriteTextures);
		glBindTexture(GL_TEXTURE_2D_ARRAY, spriteTextures);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevelCount, GL_RGBA, width, height, layerCount);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width, height, layerCount, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

		for (int i = 0; i < terrains.size(); i++) {
			int w, h, nrChannels;
			unsigned char* data = stbi_load(FileSystem::getPath(terrains[i].texturePath).c_str(), &w, &h, &nrChannels, 4);
			if (data) {
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
			}
			stbi_image_free(data);
		}
	}

	void setAllUniform(Shader& shader) {
		shader.use();

		shader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
		shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		shader.setVec3("lightPos", glm::vec3(1.0f, 0.0f, 0.0f));
		shader.setVec3("lightDirection", glm::vec3(-0.2f, -1.0f, -0.3f));

		shader.setInt("colorCount", terrains.size());
		for (int i = 0; i < terrains.size(); i++) {
			shader.setVec3("baseColor[" + std::to_string(i) + "]", terrains[i].color);
			shader.setFloat("colorStrength" +std::to_string(i) + "]", terrains[i].colorStrength);
			shader.setFloat("baseStartHeight[" + std::to_string(i) + "]", terrains[i].height);
			shader.setFloat("baseBlend[" + std::to_string(i) + "]", terrains[i].blend);
		}
		loadSpriteTexture(512, 512, terrains.size(), 1);
		shader.setInt("spriteTextures", 0);
	}

	void draw(Shader& shader, const glm::mat4& projection, const glm::mat4& view, const int &np, const glm::vec3 &cameraPos) {

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.setFloat("length", np + 1);

		shader.setFloat("minHeight", minHeight);
		shader.setFloat("maxHeight", maxHeight);

		while (!queueDraw.empty()) {
			int indx = queueDraw.front();
			queueDraw.pop();
			TerrainChunk &tc = terrainChunks[indx];
			if (tc.visible) {
				tc.visible = false;
				glBindVertexArray(tc.vao);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D_ARRAY, spriteTextures);
				glm::mat4 model(1.0f);
				model = glm::translate(model, tc.pos);
				shader.setMat4("model", model);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
				glDrawElements(GL_TRIANGLE_STRIP, indicesCount, componentType, (void*)(0));
			}
			glBindVertexArray(0);
		}
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

		printf("size indices: %ul\n", indices.size());
	}

	void GenerateHeightMapping(const std::vector<float>& heightMap, const std::vector<glm::ivec2>& indxMap, const int& offsetVertices, const int& n, const int& m, const int& width, const int& height) {

		std::vector<float> aHeight;
		for (int i = 0; i < (int)heightMap.size(); i++) {
			const glm::ivec2& coord = indxMap[i];
			if (coord.x > 0 && coord.y > 0 && coord.x <= width + 1 && coord.y <= height + 1) {
				int colIndex = i / offsetVertices;
				if (colIndex == 0 || colIndex == n - 1) continue;
				if (coord.x == 1 && i % 2 == 0) {
					aHeight.push_back(heightMap[i]);
				}
				aHeight.push_back(heightMap[i]);
				if (coord.x == m - 2 && i % 2 == 1) {
					aHeight.push_back(heightMap[i]);
				}
				if (minHeight > heightMap[i]) minHeight = heightMap[i];
				if (maxHeight < heightMap[i]) maxHeight = heightMap[i];
			}
		}

		unsigned int hVbo;
		int HeightSize = aHeight.size() * sizeof(float);
		glGenBuffers(1, &hVbo);
		glBindBuffer(GL_ARRAY_BUFFER, hVbo);
		glBufferData(GL_ARRAY_BUFFER, HeightSize, &aHeight.at(0), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	void GenerateNormalMapping(const std::vector<float>& heightMap, const std::vector<glm::ivec2>& indxMap, const int& offsetVertices, const float& halfOffset,
		const int& n, const int& m, const int& width, const int& height) {

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

		std::vector<glm::vec3> aNormal;
		for (int i = 0; i < (int)heightMap.size(); i++) {
			const glm::ivec2& coord = indxMap[i];
			if (coord.x > 0 && coord.y > 0 && coord.x <= width + 1 && coord.y <= height + 1) {
				int colIndex = i / offsetVertices;
				if (colIndex == 0 || colIndex == n - 1) continue;
				if (coord.x == 1 && i % 2 == 0) {
					aNormal.push_back(normalMap[i]);
				}
				aNormal.push_back(normalMap[i]);
				if (coord.x == m - 2 && i % 2 == 1) {
					aNormal.push_back(normalMap[i]);
				}
			}
		}

		unsigned int normVbo;
		int normSize = aNormal.size() * sizeof(float) * 3;
		glGenBuffers(1, &normVbo);
		glBindBuffer(GL_ARRAY_BUFFER, normVbo);
		glBufferData(GL_ARRAY_BUFFER, normSize, &aNormal.at(0), GL_STATIC_DRAW);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
		glEnableVertexAttribArray(1);
	}

	void generateColorTerrain() {

		Terrain region;
		region.setRegion("water", { 0.058, 0.368, 0.611 }, 0.0f, 0.0f, 0.25f, "assets/textures/Water.png");
		terrains.push_back(region);
		region.setRegion("sandy grass", { 0.941 ,0.929, 0.643 }, 0.0f, 0.3, 0.05f, "assets/textures/Sandy grass.png");
		terrains.push_back(region);
		region.setRegion("Stony ground", { 0.325, 0.82, 0.204 }, 0.0f, 0.4, 0.1f, "assets/textures/Stony ground.png");
		terrains.push_back(region);
		region.setRegion("grass", { 0.066, 0.486, 0.074 }, 0.0f, 0.5, 0.2f, "assets/textures/Grass.png");
		terrains.push_back(region);
		region.setRegion("rock 1", { 0.333, 0.254, 0.141 }, 0.0f, 0.7, 0.1f, "assets/textures/Rocks 1.png");
		terrains.push_back(region);
		region.setRegion("rock 2", { 0.235, 0.145, 0.082 }, 0.0f, 0.8, 0.1f, "assets/textures/Rocks 2.png");
		terrains.push_back(region);
		region.setRegion("snow", { 1.000, 1.000, 1.000 }, 0.0f, 0.85, 0.1f, "assets/textures/Snow.png");
		terrains.push_back(region);
	}
	
	static float calculateHeightValue(const float& v, const float& heightMultiplier) {
		return (v * heightMultiplier) - heightMultiplier;
	}


	glm::vec3 lerp(float t, glm::vec3 v0, glm::vec3 v1) {
		return v0 + (t * (v1 - v0));
	}
};

#endif
