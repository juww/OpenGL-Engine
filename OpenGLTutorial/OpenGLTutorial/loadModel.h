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

#include "animator.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

const unsigned int INF = 4294967294U;
std::map<std::string, int> vertexAttributeArray({ 
	{"POSITION",0},
	{"NORMAL",1},
	{"TEXCOORD_0",2},
	{"TANGENT",3},
	{"COLOR_0",4},
	{"JOINTS_0",5},
	{"WEIGHTS_0",6},
});

struct MaterialModel {
	bool flag;
	glm::vec4 baseColor;
	unsigned int albedoMap, normalMap, roughnessMap, emissiveMap, occlusionMap;
	float metallicFactor, roughnessFactor;

	MaterialModel() {
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
	~MaterialModel() {

	}
};

std::vector<std::pair<int, int> > vp;
std::string nameNode[1000];

struct AnimationModel {
	std::string name;
	float duration;
	std::vector<float> keyframe;
	
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

static float HexToFloat(unsigned char temp[]) {
	uint32_t x = temp[3];
	for (int i = 2; i >= 0; i--) x = (x << 8) | temp[i];
	static_assert(sizeof(float) == sizeof(uint32_t), "Float and uint32_t size dont match. Check another int type");

	float f{};
	memcpy(&f, &x, sizeof(x));

	return f;
}

class loadModel {
public:
	tinygltf::Model model;
	std::vector<glm::mat4> matrices;
	std::vector<glm::mat4> inverseMatrices;
	std::vector<glm::mat4> globalTransform;
	std::map<int, unsigned int> ebos;
	std::map<int, unsigned int> vbos;
	std::map<int, unsigned int> vaos;
	std::vector<MaterialModel> materials;
	std::vector<Animator> animator;

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
		ret = loadAnimation();

		tinygltf::Skin& skin = model.skins[0];
		for (int i = 0; i < (int)vp.size(); i++) {
			int a = 0, b=0;
			for (int j = 0; j < (int)skin.joints.size(); j++) {
				int joint = skin.joints[j];
				if (vp[i].first == joint) {
					a = 1;
				}
				if (vp[i].second == joint) {
					b = 1;
				}
			}
			if (a+b == 2) {
				printf("%d %d\n", vp[i].first, vp[i].second);
			}
		}

		for (int j = 0; j < (int)skin.joints.size(); j++) {
			int joint = skin.joints[j];
			printf("%d -> %s\n", joint, nameNode[joint].c_str());
		}
	}

	void DrawModel(Shader &shader) {

		int defaultScene = model.defaultScene < 0 ? 0 : model.defaultScene;
		tinygltf::Scene& scene = model.scenes[defaultScene];
		for (int i = 0; i < (int)model.skins[0].joints.size(); i++) {
			int joint = model.skins[0].joints[i];
			globalTransform[i] = matrices[joint] * inverseMatrices[i];
			shader.setMat4("boneTransform[" + std::to_string(i) + "]", globalTransform[i]);
		}
		for (int node : scene.nodes) {
			drawNodes(node, shader);
		}
	}

	void localTransform(tinygltf::Node& node, glm::mat4& matrix) {

		glm::vec3 s(1.0f);
		glm::vec3 t(0.0f);
		glm::quat q(1.0f, 0.0f, 0.0f, 0.0f);

		if (node.matrix.size() != 0) {
			for (int i = 0; i < 4; i++) {
				glm::vec4 temp;
				for (int j = 0; j < 4; j++) {
					temp[j] = node.matrix[(i * 4) + j];
				}
				matrix[i] = temp;
			}
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

		std::cout << glm::to_string(matrix) << std::endl;
	}

private:

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

			unsigned int offsetofData = bufferView.byteOffset + accessor.byteOffset;
			unsigned int stride = accessor.ByteStride(bufferView);
			unsigned int lengthOfData = accessor.count * stride;

			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, lengthOfData, &buffer.data.at(offsetofData), GL_STATIC_DRAW);
			if (vaa->first == "JOINTS_0") {
				glVertexAttribIPointer(vaa->second, accessor.type, accessor.componentType,
					accessor.ByteStride(bufferView), (void*)(0));
			} else {
				glVertexAttribPointer(vaa->second, accessor.type, accessor.componentType,
					accessor.normalized ? GL_TRUE : GL_FALSE, accessor.ByteStride(bufferView), (void*)(0));
			}
			glEnableVertexAttribArray(vaa->second);
		}
	}

	void bindMesh(int indx) {
		if (indx < 0 || indx >= (int)model.meshes.size()){
			return;
		}

		tinygltf::Mesh& mesh = model.meshes[indx];
		printf("---------------------------------\n");
		printf("Mesh ke - %d\n", indx);
		printf("mesh name: %s\n", mesh.name.c_str());

		unsigned int vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		vaos[indx] = vao;

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

			if (ebos[prim.indices]) {
				std::cout << "already bind the mesh" << std::endl;
				continue;
			}
			unsigned int ebo;
			glGenBuffers(1, &ebo);
			unsigned int offsetofData = bufferView.byteOffset + accessor.byteOffset;
			unsigned int stride = accessor.ByteStride(bufferView);
			unsigned int lengthOfData = accessor.count * stride;

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, lengthOfData, &buffer.data.at(offsetofData), GL_STATIC_DRAW);
			ebos[prim.indices] = ebo;

			bindAttributeIndex(prim);
			bindMaterial(prim.material);
		}

		glBindVertexArray(0);
	}

	void bindSkin(int indx) {

		if (indx == -1) return;

		tinygltf::Skin& skin = model.skins[indx];
		printf("skin name ---- %s\n", skin.name.c_str());
		if (skin.inverseBindMatrices < 0 || skin.inverseBindMatrices >= model.accessors.size()) {
			printf("E| Skin Inverse Bind Matrices is not found!\n");
			return;
		}
		tinygltf::Accessor& accessor = model.accessors[skin.inverseBindMatrices];
		if (accessor.bufferView < 0 || accessor.bufferView >= model.bufferViews.size()) {
			printf("E| Skin accessor bufferView is not found!\n");
			return;
		}
		if (accessor.type != 36) {
			printf("E| error Skin Type data!\nE| accessor.type = %d\n", accessor.type);
			return;
		}
		tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
		if (bufferView.buffer < 0 || bufferView.buffer >= model.buffers.size()) {
			printf("E| Skin buffer data is not found!\n");
			return;
		}
		tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
		
		unsigned int offsetofData = bufferView.byteOffset + accessor.byteOffset;
		unsigned int stride = accessor.ByteStride(bufferView);
		unsigned int lengthOfData = accessor.count * stride;

		int cnt = 0, mi = 0;
		unsigned char temp[4];
		for (int i = offsetofData; i < offsetofData + lengthOfData; i++) {
			//if (cnt > 128) break;
			temp[cnt % 4] = buffer.data[i];
			printf("%3d ", buffer.data[i]);
			if (cnt % 4 == 3) {
				float a = HexToFloat(temp);
				inverseMatrices[cnt / 64][mi / 4][mi % 4] = a;
				printf(" - %f #  ", a);
				mi++;
			}
			if (cnt % 16 == 15) printf("\n");
			if (cnt % 64 == 63) {
				mi = 0;
				printf("indx %d ============================\n", cnt/64);
				std::cout << to_string(inverseMatrices[(cnt / 64)]) << "\n";
			}
			cnt++;
		};

		printf("skin accessor %s\n", accessor.name.c_str());
		printf("bufferView = %d\n", accessor.bufferView);
		printf("byteoffset = %d\n", accessor.byteOffset);
		printf("componentType = %d\n", accessor.componentType);
		printf("Type = %d\n", accessor.type);
		printf("bytestride = %d\n", accessor.ByteStride(model.bufferViews[accessor.bufferView]));
		printf("size count = %d\n", accessor.count);
		//std::string name;
		//int skeleton{ -1 };             // The index of the node used as a skeleton root
		//std::vector<int> joints;      // Indices of skeleton nodes

	}

	void bindNodes(int indx, int parent = -1) {
		if (indx < 0 || indx >= (int)model.nodes.size()) {
			std::cout << "index node is out of bound\n";
			std::cout << "indx: "<<indx<<"\n";
			return;
		}
		tinygltf::Node& node = model.nodes[indx];
		printf("node[%d] name: %s\n", indx, node.name.c_str());

		localTransform(node,matrices[indx]);
		if (parent != -1) {
			vp.push_back({ parent,indx });
			matrices[indx] = matrices[indx] * matrices[parent];
			//inverseMatrices[indx] = glm::inverse(matrices[indx]);
		}
		nameNode[indx] = node.name;
		bindMesh(node.mesh);
		bindSkin(node.skin);

		for (int i = 0; i < node.children.size(); i++) {
			bindNodes(node.children[i], indx);
		}

		//int light{ -1 };    // light source index (KHR_lights_punctual)
		//int emitter{ -1 };  // audio emitter index (KHR_audio)
		//std::vector<double> weights;  // The weights of the instantiated Morph Target
	}

	bool loadScene() {
		bool ret = false;

		matrices.resize(model.nodes.size(), glm::mat4(1.0));
		inverseMatrices.resize(model.nodes.size(), glm::mat4(1.0));
		globalTransform.resize(model.nodes.size(), glm::mat4(1.0));
		materials.resize(model.materials.size(), MaterialModel());
		animator.resize(model.animations.size());

		int defaultScene = model.defaultScene < 0 ? 0 : model.defaultScene;
		tinygltf::Scene& scene = model.scenes[defaultScene];
		for (int node : scene.nodes) {
			bindNodes(node);
		}

		return ret;
	}


	bool loadAnimation() {
		bool ret = false;

		int current_animation = 0;
		for (tinygltf::Animation& animation : model.animations) {
			printf("%d: %s\n", current_animation, animation.name.c_str());

			int sampler_length = animation.samplers.size();
			int channel_length = animation.channels.size();
			for (tinygltf::AnimationChannel& channel: animation.channels) {

				if (current_animation > 3) break;
				int indxSampler = channel.sampler;
				int targetNode = channel.target_node;

				printf("target node animation: %d\n", targetNode);

				std::string targetPath = channel.target_path;
				if (indxSampler < 0 || indxSampler >= sampler_length) {
					printf("E| animation index sampler is not found!\n");
					printf("E| indxSampler: %d\n", indxSampler);
					break;
				}
				tinygltf::AnimationSampler& sampler = animation.samplers[indxSampler];
				tinygltf::Accessor& accessorInput = model.accessors[sampler.input];
				tinygltf::BufferView& bufferViewInput = model.bufferViews[accessorInput.bufferView];
				tinygltf::Buffer& bufferInput = model.buffers[bufferViewInput.buffer];

				if (accessorInput.type != 65 || accessorInput.componentType != GL_FLOAT) {
					printf("E| animation input is not correct!\n");
					printf("E| accessor type: %d\n", accessorInput.type);
					printf("E| accessor ComponentType: %d\n", accessorInput.componentType);
					break;
				}

				unsigned int offsetofData = bufferViewInput.byteOffset + accessorInput.byteOffset;
				unsigned int stride = accessorInput.ByteStride(bufferViewInput);
				unsigned int lengthOfData = accessorInput.count * stride;

				int cnt = 0, mi = 0;
				unsigned char tempBuffer[4];
				printf("timestamp:\n");
				std::vector<float> inputData;
				for (unsigned int i = offsetofData; i < offsetofData + lengthOfData; i++) {
					tempBuffer[cnt % 4] = bufferInput.data[i];
					if (cnt % 4 == 3) {
						float timestamp = HexToFloat(tempBuffer);
						printf("%.7f\n", timestamp);
						inputData.push_back(timestamp);
					}
					cnt++;
				}
				printf("\n");
				tinygltf::Accessor& accessorOutput = model.accessors[sampler.output];
				tinygltf::BufferView& bufferViewOutput = model.bufferViews[accessorOutput.bufferView];
				tinygltf::Buffer& bufferOutput = model.buffers[bufferViewOutput.buffer];

				offsetofData = bufferViewOutput.byteOffset + accessorOutput.byteOffset;
				stride = accessorOutput.ByteStride(bufferViewOutput);
				lengthOfData = accessorOutput.count * stride;

				std::cout << "stride " << stride << "\n";

				cnt = 0; mi = 0;
				int vtemp = 0;
				printf("output %s:\n", channel.target_path.c_str());
				std::vector<glm::vec4> outputData;
				glm::vec4 tempTransform(0.0f);
				for (unsigned int i = offsetofData; i < offsetofData + lengthOfData; i++) {
					tempBuffer[cnt % 4] = bufferInput.data[i];
					if (cnt % 4 == 3) {
						float tempdata = HexToFloat(tempBuffer);
						tempTransform[vtemp] = tempdata;
						vtemp++;
					}
					if (cnt % stride == stride - 1) {
						vtemp = 0;
						outputData.push_back(tempTransform);
					}
					cnt++;
				}
				animator[current_animation].addKeyframe(inputData, outputData, targetNode,sampler.interpolation, targetPath);
			}
			
			//std::vector<AnimationChannel> channels;
			int sampler{ -1 };          // required
			int target_node{ -1 };      // optional index of the node to target (alternative
										// target should be provided by extension)
			std::string target_path;	// required with standard values of ["translation",
										// "rotation", "scale", "weights"]

			//std::vector<AnimationSampler> samplers;
			int input{ -1 };			// indx to accessor (?) time keyframe
			int output{ -1 };			// indx to accessor (?) transform{translate,rotation, scale};

			std::string interpolation;	// "LINEAR", "STEP","CUBICSPLINE" or user defined
										// string. default "LINEAR"

			current_animation++;
		}


		return ret;
	}

	void drawMesh(int indx, Shader& shader) {
		if (indx < 0 || indx >= (int)model.meshes.size()) {
			return;
		}
		tinygltf::Mesh& mesh = model.meshes[indx];
		glBindVertexArray(vaos[indx]);

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

			if (prim.material != -1) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, materials[prim.material].albedoMap);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, materials[prim.material].normalMap);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, materials[prim.material].roughnessMap);
			}

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebos[prim.indices]);
			glDrawElements(prim.mode, accessor.count, accessor.componentType, (void*)(0));
		}
		
		glBindVertexArray(0);
	}
	
	void drawNodes(int indx, Shader& shader) {
		if (indx < 0 || indx >= (int)model.nodes.size()) {
			return;
		}
		tinygltf::Node& node = model.nodes[indx];
		//glm::mat4 mat = matrices[indx];
		glm::mat4 mat(1.0f);
		//mat = glm::translate(mat, { 0.0f, -20.0f, 0.0f });
		//model = glm::rotate(model, (float)glfwGetTime(), { 1.0f,0.0f,0.0f });
		//mat = glm::scale(mat, { 0.01f,0.01f,0.01f });
		 //std::cout << glm::to_string(mat) << "\n";
		shader.setMat4("model", mat);

		drawMesh(node.mesh, shader);

		for (int i = 0; i < node.children.size(); i++) {
			drawNodes(node.children[i], shader);
		}

	}

};

#endif