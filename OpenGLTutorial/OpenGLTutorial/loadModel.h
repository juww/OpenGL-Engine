#pragma once
#ifndef LOAD_MODEL_H
#define LOAD_MODEL_H

#include "tinyGLTF/tiny_gltf.h"
#include "tinyGLTF/stb_image.h"
#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"

#include <iostream>
#include <cstring>
#include <string>
#include <map>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

const unsigned int INF = 4294967294U;
std::map<std::string, int> vertexAttributeArray({ 
	{"POSITION",0},
	{"NORMAL",1},
	{"TEXCOORD_0",2},
	{"TANGENT",3},
	{"COLOR_0",4}
});

struct Materials {
	bool flag;
	glm::vec4 baseColor;
	unsigned int albedoMap, normalMap, roughnessMap, emissiveMap, occlusionMap;
	float metallicFactor, roughnessFactor;

	Materials() {
		flag = false;
		baseColor = glm::vec4(0.0);
		albedoMap = 0;
		normalMap = 0;
		roughnessMap = 0;
		emissiveMap = 0;
		occlusionMap = 0;
		metallicFactor = 0.0f;
		roughnessFactor = 0.0f;
	}
	~Materials() {

	}
};

GLenum glCheckError_(const char* file, int line)
{
	std::cout << "check error" << std::endl;
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

class loadModel {
public:
	tinygltf::Model model;
	std::vector<glm::mat4> matrices;
	std::map<int, unsigned int> ebos;
	std::map<int, unsigned int> vbos;
	unsigned int vao;
	std::vector<Materials> materials;

	loadModel(const char* filename) {
		bool ret = false;
		std::string err;
		std::string warn;
		tinygltf::TinyGLTF loader;

		if (strstr(filename, ".gltf") != NULL) {
			ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
		}

		if (strstr(filename, ".glb") != NULL) {
			ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename); // for binary glTF(.glb)
		}

		if (!warn.empty()) {
			printf("Warn: %s\n", warn.c_str());
		}

		if (!err.empty()) {
			printf("Err: %s\n", err.c_str());
		}

		if (!ret) {
			printf("Failed to parse glTF\n");
			return;
		}
		printf("Loaded glTF: %s\n", filename);
		ret = loadScene();
	}

	void DrawModel(Shader &shader) {

		glBindVertexArray(vao);

		int defaultScene = model.defaultScene < 0 ? 0 : model.defaultScene;
		tinygltf::Scene& scene = model.scenes[defaultScene];
		for (int node : scene.nodes) {
			drawNodes(node, shader);
		}
		glBindVertexArray(0);
	}

private:

	void localTransform(tinygltf::Node& node, glm::mat4 &matrix) {

		glm::vec3 s(1.0f);
		glm::vec3 t(0.0f);
		glm::quat q(1.0f, 0.0f, 0.0f, 0.0f);

		if (node.matrix.size() != 0) {
			for (int i = 0; i < 4; i++) {
				glm::vec4 t;
				for (int j = 0; j < 4; j++) {
					t[j] = node.matrix[(i * 4) + j];
					std::cout << node.matrix[(i * 4)+(j)] << " ";
				}
				std::cout << "\n";
				matrix[i] = t;
			}
			std::cout << "\n";
			//return;
		}
		// M = T * R * S
		if (!node.translation.empty()) {
			for (int i = 0; i < node.translation.size(); i++) {
				t[i] = node.translation[i];
			}
			matrix = glm::translate(matrix, t);
		}

		if (!node.rotation.empty()) {
			for (int i = 0; i < node.rotation.size(); i++) {
				q[(i + 1) % 4] = node.rotation[i];
			}
			glm::mat4 rotMatrix = glm::mat4_cast(q);
			matrix = matrix * rotMatrix;
		}

		if (!node.scale.empty()) {
			for (int i = 0; i < node.scale.size(); i++) {
				s[i] = node.scale[i];
			}
			matrix = glm::scale(matrix, s);
		}
	}

	unsigned int loadTexture(int indx) {

		if (indx == -1) return 0;

		unsigned int textureID;
		if (indx < 0 || indx >= (int)model.textures.size()) {
			std::cout << "index textures is out of bound\n";
			std::cout << "indx: "<<indx<<"\n";
			return 0;
		}
		tinygltf::Texture& t = model.textures[indx];
		if (t.source < 0 || t.source >= (int)model.images.size()) {
			std::cout << "index source image is out of bound\n";
			std::cout << "indx: " << t.source << "\n";
			return 0;
		}
		tinygltf::Image& image = model.images[t.source];
		if (t.sampler < 0 || t.sampler >= (int)model.samplers.size()) {
			std::cout << "index sampler is out of bound\n";
			std::cout << "indx: " << t.sampler << "\n";
			return 0;
		}
		tinygltf::Sampler& sampler = model.samplers[t.sampler];

		unsigned int format = 1;
		switch (image.component) {
		case 1:
			format = GL_RED;
			break;
		case 3:
			format = GL_RGB;
			break;
		case 4:
			format = GL_RGBA;
			break;
		default:
			break;
		}
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler.minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler.magFilter);
		glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, format, image.pixel_type, &image.image.at(0));
		//glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		return textureID;
	}

	void bindMaterial(int indx) {
		if (indx < 0 || indx >= (int)model.materials.size()) {
			std::cout << "index material is out of bound\n";
			std::cout << "indx: " << indx << "\n";
			return;
		}
		if (materials[indx].flag == true) {
			return;
		}
		tinygltf::Material& material = model.materials[indx];
		
		std::vector<double>& colorFactor = material.pbrMetallicRoughness.baseColorFactor;
		for (int i = 0; i < colorFactor.size(); i++) {
			materials[indx].baseColor[i] = colorFactor[i];
		}
		materials[indx].metallicFactor = material.pbrMetallicRoughness.metallicFactor;
		materials[indx].roughnessFactor = material.pbrMetallicRoughness.roughnessFactor;

		materials[indx].albedoMap = loadTexture(material.pbrMetallicRoughness.baseColorTexture.index);
		materials[indx].normalMap = loadTexture(material.normalTexture.index);
		materials[indx].roughnessMap = loadTexture(material.pbrMetallicRoughness.metallicRoughnessTexture.index);
		materials[indx].emissiveMap = loadTexture(material.emissiveTexture.index);
		materials[indx].occlusionMap = loadTexture(material.occlusionTexture.index);
		materials[indx].flag = true;
	}

	void bindAttributeIndex(tinygltf::Primitive& prim) {
		for (auto& attr : prim.attributes) {
			std::cout << attr.first << " - " << attr.second << "\n";
			tinygltf::Accessor& accessor = model.accessors[attr.second];
			if (accessor.bufferView < 0 || accessor.bufferView >= (int)model.bufferViews.size()) {
				std::cout << "index bufferView is out of bound\n";
				std::cout << "indx: " << accessor.bufferView << "\n";
				continue;
			}
			tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
			if (bufferView.buffer < 0 || bufferView.buffer >= (int)model.buffers.size()) {
				std::cout << "index buffer is out of bound\n";
				std::cout << "indx: " << bufferView.buffer << "\n";
				continue;
			}
			tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

			if (bufferView.target == 0) {
				std::cout << "WARN: bufferView.target is zero\n";
				continue;
			}

			auto vaa = vertexAttributeArray.find(attr.first);
			if (vaa == vertexAttributeArray.end()) {
				std::cout << "attribute is not found!\n";
				std::cout << attr.first << " " << attr.second << std::endl;
				continue;
			}
			unsigned int vbo;
			glGenBuffers(1, &vbo);
			vbos[attr.second] = vbo;
			printf("vbo %u\n", vbo);

			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, bufferView.byteLength,
				&buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
			printf("bufferView.byteLength = %d\n", bufferView.byteLength);
			printf("bufferView.byteOffset = %d\n", bufferView.byteOffset);

			printf("vaa -> %s: %d\n", vaa->first.c_str(), vaa->second);
			printf("accessor.type ==== %d\n", accessor.type);
			printf("accessor.componentType ==== %d\n", accessor.componentType);
			printf("accessor.ByteStride(bufferView) ==== %d\n", accessor.ByteStride(bufferView));
			printf("accessor.byteOffset ==== %d\n", accessor.byteOffset);
			glEnableVertexAttribArray(vaa->second);
			glVertexAttribPointer(vaa->second, accessor.type, accessor.componentType,
				accessor.normalized ? GL_TRUE : GL_FALSE, accessor.ByteStride(bufferView), (void*)(0 + accessor.byteOffset));
		}
	}

	void bindMesh(int indx) {
		if (indx < 0 || indx >= (int)model.meshes.size()){
			std::cout << "index mesh is out of bound\n";
			std::cout << "indx: " << indx << "\n";
			return;
		}
		tinygltf::Mesh& mesh = model.meshes[indx];
		printf("---------------------------------\n");
		printf("Mesh ke - %d\n", indx);
		printf("mesh name: %s\n", mesh.name.c_str());

		for (int i = 0; i < mesh.primitives.size(); i++) {
			tinygltf::Primitive &prim = mesh.primitives[i];
			if (prim.indices < 0 || prim.indices >= (int)model.accessors.size()) {
				std::cout << "index indices of accessor is out of bound\n";
				std::cout << "indx: " << prim.indices << "\n";
				return;
			}
			tinygltf::Accessor &accessor = model.accessors[prim.indices];
			if (accessor.bufferView < 0 || accessor.bufferView >= (int)model.bufferViews.size()) {
				std::cout << "index bufferView is out of bound\n";
				std::cout << "indx: " << accessor.bufferView << "\n";
				return;
			}
			tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
			if (bufferView.buffer < 0 || bufferView.buffer >= (int)model.buffers.size()) {
				std::cout << "index buffer is out of bound\n";
				std::cout << "indx: " << bufferView.buffer << "\n";
				return;
			}
			tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

			printf("primitive[%d]\n", i);
			printf("prim.indices = %d\n", prim.indices);
			printf("accessor.bufferView = %d\n", accessor.bufferView);
			printf("bufferView.buffer = %d\n", bufferView.buffer);
			
			printf("buffer data size %d\n", buffer.data.size());
			if (ebos[prim.indices]) {
				std::cout << "already bind the mesh" << std::endl;
				continue;
			}
			unsigned int ebo;
			glGenBuffers(1, &ebo);
			printf("generate buffer EBO ------------------------ %u\n", ebo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferView.byteLength,
				&buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);

			ebos[prim.indices] = ebo;
			printf("ebos[%d] = %u\n", prim.indices, ebos[prim.indices]);
			printf("bufferView.byteLength = %d\n", bufferView.byteLength);
			printf("bufferView.byteOffset = %d\n", bufferView.byteOffset);

			bindAttributeIndex(prim);
			
			bindMaterial(prim.material);
		}
	}

	void bindNodes(int indx) {
		if (indx < 0 || indx >= (int)model.nodes.size()) {
			std::cout << "index node is out of bound\n";
			std::cout << "indx: "<<indx<<"\n";
			return;
		}
		tinygltf::Node& node = model.nodes[indx];
		printf("node[%d] name: %s\n", indx, node.name.c_str());

		localTransform(node,matrices[indx]);
		bindMesh(node.mesh);

		for (int i = 0; i < node.children.size(); i++) {
			bindNodes(node.children[i]);
		}
		//int skin{ -1 };
		//int light{ -1 };    // light source index (KHR_lights_punctual)
		//int emitter{ -1 };  // audio emitter index (KHR_audio)
		//std::vector<double> weights;  // The weights of the instantiated Morph Target
	}

	bool loadScene() {
		bool ret = false;

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		ret = (vao != 0);

		matrices.resize(model.nodes.size(), glm::mat4(1.0));
		materials.resize(model.materials.size(), Materials());

		int defaultScene = model.defaultScene < 0 ? 0 : model.defaultScene;
		tinygltf::Scene& scene = model.scenes[defaultScene];
		for (int node : scene.nodes) {
			bindNodes(node);
		}
		glBindVertexArray(0);

		return ret;
	}

	void drawMesh(int indx, Shader& shader) {
		if (indx < 0 || indx >= (int)model.meshes.size()) {
			return;
		}
		tinygltf::Mesh& mesh = model.meshes[indx];

		for (int i = 0; i < mesh.primitives.size(); i++) {
			tinygltf::Primitive &prim = mesh.primitives[i];
			if (prim.indices < 0 || prim.indices >= (int)model.accessors.size()) {
				continue;
			}
			tinygltf::Accessor& accessor = model.accessors[prim.indices];
			if (accessor.bufferView < 0 || accessor.bufferView >= (int)model.bufferViews.size()) {
				continue;
			}
			tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
			if (bufferView.buffer < 0 || bufferView.buffer >= (int)model.buffers.size()) {
				continue;
			}

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, materials[prim.material].albedoMap);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, materials[prim.material].normalMap);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, materials[prim.material].roughnessMap);

			//std::map<int, unsigned int>::iterator temp = ebos.find(prim.indices);
			//printf("prim index = %d\n", prim.indices);
			// printf("ebos[%d] = %d\n", prim.indices, ebos[prim.indices]);
			//if (temp == ebos.end()) {
			//	std::cout << "asu" << std::endl;
			//	continue;
			//}
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebos[prim.indices]);
			glDrawElements(prim.mode, accessor.count, accessor.componentType, (void*)(0 + accessor.byteOffset));
		}
		
	}

	void drawNodes(int indx, Shader& shader) {
		if (indx < 0 || indx >= (int)model.nodes.size()) {
			return;
		}
		tinygltf::Node& node = model.nodes[indx];
		glm::mat4 mat = matrices[indx];
		//glm::mat4 mat(1.0f);
		//mat = glm::translate(mat, { 0.0f, 0.0f, 5.0f });
		//model = glm::rotate(model, (float)glfwGetTime(), { 1.0f,0.0f,0.0f });
		//mat = glm::scale(mat, { 0.1f,0.1f,0.1f });
		 //std::cout << glm::to_string(mat) << "\n";
		shader.setMat4("model", mat);

		drawMesh(node.mesh, shader);

		for (int i = 0; i < node.children.size(); i++) {
			drawNodes(node.children[i], shader);
		}

	}

};

#endif