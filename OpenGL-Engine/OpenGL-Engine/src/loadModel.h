#pragma once
#ifndef LOAD_MODEL_H
#define LOAD_MODEL_H

#include "tinyGLTF/tiny_gltf.h"
#include "tinyGLTF/stb_image.h"
#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"
#include <glm/gtx/quaternion.hpp>

#include <iostream>
#include <cstring>
#include <string>
#include <map>

#include "animator.h"

const unsigned int INF = 4294967294U;
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

class loadModel {
public:
	tinygltf::Model model;
	std::vector<glm::mat4> localMatrices;
	std::vector<glm::mat4> modelMatrices;
	std::vector<glm::mat4> inverseMatrices;
	std::vector<glm::mat4> animationTransform;
	std::map<int, unsigned int> ebos;
	std::map<int, unsigned int> vbos;
	std::map<int, unsigned int> vaos;
	std::vector<MaterialModel> materials;
	Animator animator;

	glm::vec3 pos = glm::vec3(0.0f);
	glm::vec3 rot = glm::vec3(1.0f);
	float angle = 0.0f;
	glm::vec3 scale = glm::vec3(1.0f);


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

        setDictAttribArray();
		ret = loadScene();
		ret = loadAnimation();

        glBindVertexArray(0);
	}

    void updateSkeletalNode(int indx, int parent, Shader* shader) {
        if (indx < 0 || indx >= (int)model.nodes.size()) {
            return;
        }
        tinygltf::Node& node = model.nodes[indx];
        animationTransform[indx] = glm::mat4(1.0f);

        std::map<int, Transformation>::iterator itr = animator.currentPose.find(indx);
        if (itr == animator.currentPose.end()) {
            Transformation tmp;
            animator.currentPose.insert({ indx, tmp });
            itr = animator.currentPose.find(indx);
        }
        Transformation& temp = itr->second;

        animationTransform[indx] = glm::scale(animationTransform[indx], temp.scalation);

        glm::quat q(temp.rotation[0], temp.rotation[1], temp.rotation[2], temp.rotation[3]);
        glm::mat4 rot = glm::toMat4(q);
        animationTransform[indx] = rot * animationTransform[indx];

        animationTransform[indx] = glm::translate(animationTransform[indx], temp.translation);

        if (parent != -1) {
            animationTransform[indx] = animationTransform[parent] * animationTransform[indx];
        }
        for (int i = 0; i < node.children.size(); i++) {
            updateSkeletalNode(node.children[i], indx, shader);
        }
    }

	void update(Shader* shader, float deltaTime) {

        if (!animator.update(deltaTime)) {
            return;
        }

        int defaultScene = model.defaultScene < 0 ? 0 : model.defaultScene;
        tinygltf::Scene& scene = model.scenes[defaultScene];
        for (int node : scene.nodes) {
            updateSkeletalNode(node, -1, shader);
        }
        for (int k = 0; k < (int)model.skins.size(); k++) {
            for (int i = 0; i < (int)model.skins[k].joints.size(); i++) {
                int joint = model.skins[k].joints[i];
                //animationTransform[joint] = animationTransform[joint] * glm::inverse(modelMatrices[joint]);
                animationTransform[joint] = animationTransform[joint] * inverseMatrices[joint];

                shader->setMat4("boneTransform[" + std::to_string(i) + "]", animationTransform[joint]);
            }
		}
	}

	void DrawModel(Shader *shader) {

		shader->use();
		int defaultScene = model.defaultScene < 0 ? 0 : model.defaultScene;
		tinygltf::Scene& scene = model.scenes[defaultScene];
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
		if (!node.scale.empty()) {
			for (int i = 0; i < node.scale.size(); i++) {
				s[i] = node.scale[i];
			}
			matrix = glm::scale(matrix, s);
		}

		if (!node.rotation.empty()) {
			for (int i = 0; i < node.rotation.size(); i++) {
				q[(i + 1) % 4] = node.rotation[i];
			}
			glm::mat4 rotMatrix = glm::mat4_cast(q);
			matrix = matrix * rotMatrix;
		}

		if (!node.translation.empty()) {
			for (int i = 0; i < node.translation.size(); i++) {
				t[i] = node.translation[i];
			}
			matrix = glm::translate(matrix, t);
		}
		//std::cout << glm::to_string(matrix) << std::endl;
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
		tinygltf::Sampler sampler;
		if (t.sampler < 0 || t.sampler >= (int)model.samplers.size()) {
			std::cout << "index sampler is out of bound\n";
			std::cout << "indx: " << t.sampler << "\n";
			sampler.wrapS = GL_REPEAT;
			sampler.wrapT = GL_REPEAT;
			sampler.minFilter = GL_LINEAR;
			sampler.magFilter = GL_LINEAR;
		} else {
			sampler = model.samplers[t.sampler];
		}

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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrapT);
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

			auto vaa = dictVertexAttributeArray.find(attr.first);
			if (vaa == dictVertexAttributeArray.end()) {
				std::cout << "attribute is not found!\n";
				std::cout << attr.first << " " << attr.second << std::endl;
				continue;
			}
			unsigned int vbo;
			glGenBuffers(1, &vbo);
			vbos[attr.second] = vbo;
			//printf("vbo %u\n", vbo);

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

        for (size_t j = 0; j < model.skins[indx].joints.size(); ++j) {
            glm::mat4 mt = glm::make_mat4((float*)(buffer.data.data() + offsetofData + stride * j));
            int joint = model.skins[indx].joints[j];
            inverseMatrices[joint] = mt;
            tinygltf::Node& nodeTemp = model.nodes[joint];
            printf("joint %d : %s\n", joint, nodeTemp.name.c_str());
            std::cout << to_string(mt) << "\n";
        }
	}

	void bindNodes(int indx, int parent = -1) {
		if (indx < 0 || indx >= (int)model.nodes.size()) {
			std::cout << "index node is out of bound\n";
			std::cout << "indx: "<<indx<<"\n";
			return;
		}
		tinygltf::Node& node = model.nodes[indx];
		printf("node[%d] name: %s\n", indx, node.name.c_str());

		localTransform(node,localMatrices[indx]);
		modelMatrices[indx] = localMatrices[indx];
		if (parent != -1) {
			vp.push_back({ parent,indx });
			modelMatrices[indx] = modelMatrices[parent] * modelMatrices[indx];
			//inverseMatrices[indx] = glm::inverse(modelMatrices[indx]);
		}
		nameNode[indx] = node.name;
		bindMesh(node.mesh);
		bindSkin(node.skin);

		for (int i = 0; i < node.children.size(); i++) {
			bindNodes(node.children[i], indx);
		}
	}

	bool loadScene() {
		bool ret = false;

		localMatrices.resize(model.nodes.size(), glm::mat4(1.0));
		modelMatrices.resize(model.nodes.size(), glm::mat4(1.0));
		inverseMatrices.resize(model.nodes.size(), glm::mat4(1.0));
		animationTransform.resize(model.nodes.size(), glm::mat4(1.0));
		materials.resize(model.materials.size(), MaterialModel());

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
		animator.animations.resize(model.animations.size());
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
				//printf("timestamp:\n");
				std::vector<float> inputData;
				for (unsigned int i = offsetofData; i < offsetofData + lengthOfData; i++) {
					tempBuffer[cnt % 4] = bufferInput.data[i];
					if (cnt % 4 == 3) {
						float timestamp = HexToFloat(tempBuffer);
						//printf("%.7f\n", timestamp);
						inputData.push_back(timestamp);
					}
					cnt++;
				}
				//printf("\n");
				tinygltf::Accessor& accessorOutput = model.accessors[sampler.output];
				tinygltf::BufferView& bufferViewOutput = model.bufferViews[accessorOutput.bufferView];
				tinygltf::Buffer& bufferOutput = model.buffers[bufferViewOutput.buffer];

				offsetofData = bufferViewOutput.byteOffset + accessorOutput.byteOffset;
				stride = accessorOutput.ByteStride(bufferViewOutput);
				lengthOfData = accessorOutput.count * stride;

				//std::cout << "stride " << stride << "\n";

				cnt = 0; mi = 0;
				int vtemp = 0;
				//printf("output %s:\n", channel.target_path.c_str());
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
				animator.animations[current_animation].addKeyframe(inputData, outputData, targetNode,sampler.interpolation, targetPath);
			}

			current_animation++;
		}


		return ret;
	}

	void drawMesh(int indx, Shader* shader) {
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
	
	void drawNodes(int indx, Shader* shader) {
		if (indx < 0 || indx >= (int)model.nodes.size()) {
			return;
		}
		tinygltf::Node& node = model.nodes[indx];
		glm::mat4 mat = modelMatrices[indx];
		//glm::mat4 mat(1.0f);
		mat = glm::translate(mat, pos);
		//model = glm::rotate(model, (float)glfwGetTime(), { 1.0f,0.0f,0.0f });
		mat = glm::rotate(mat, angle, rot);
        scale = glm::vec3(0.01f);
		mat = glm::scale(mat, scale);
		 //std::cout << glm::to_string(mat) << "\n";
        shader->setMat4("model", mat);

		drawMesh(node.mesh, shader);

		for (int i = 0; i < node.children.size(); i++) {
			drawNodes(node.children[i], shader);
		}

	}
private:

    std::vector<std::pair<int, int> > vp;
    std::string nameNode[1000];

    std::map<std::string, int> dictVertexAttributeArray;

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
    void setDictAttribArray() {
        dictVertexAttributeArray["POSITION"] = 0;
        dictVertexAttributeArray["NORMAL"] = 1;
        dictVertexAttributeArray["TEXCOORD_0"] = 2;
        dictVertexAttributeArray["TANGENT"] = 3;
        dictVertexAttributeArray["COLOR_0"] = 4;
        dictVertexAttributeArray["JOINTS_0"] = 5;
        dictVertexAttributeArray["WEIGHTS_0"] = 6;
    }
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

};

#endif