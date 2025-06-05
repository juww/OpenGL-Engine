#pragma once
#ifndef LOAD_MODEL_H
#define LOAD_MODEL_H

#include <TinyGLTF/tiny_gltf.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "material.h"
#include "transformation.h"

namespace gltf {

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define glCheckError() glCheckError_(__FILE__, __LINE__)
    static float HexToFloat(unsigned char temp[]);

    tinygltf::Model *tinygltf_model = nullptr;
    std::vector < std::vector<unsigned int> >nodeHierarchy;
    std::map<std::string, int> dictVertexAttributeArray = {
        {"POSITION", 0},
        {"NORMAL", 1},
        {"TEXCOORD_0", 2},
        {"TANGENT", 3},
        {"BITANGENT", 4},
        {"JOINTS_0", 5},
        {"WEIGHTS_0", 6}
    };

    void setNodeTransform(tinygltf::Node& node, Transformation& transform) {

        if (node.matrix.size() != 0) {
            for (int i = 0; i < 4; i++) {
                glm::vec4 temp{};
                for (int j = 0; j < 4; j++) {
                    temp[j] = node.matrix[(i * 4) + j];
                }
                transform.matrix[i] = temp;
            }
        }

        if (!node.scale.empty()) {
            for (int i = 0; i < node.scale.size(); i++) {
                transform.scale[i] = node.scale[i];
            }
        }

        if (!node.rotation.empty()) {
            for (int i = 0; i < node.rotation.size(); i++) {
                transform.quaternion[(i + 1) % 4] = node.rotation[i];
            }
        }

        if (!node.translation.empty()) {
            for (int i = 0; i < node.translation.size(); i++) {
                transform.pos[i] = node.translation[i];
            }
        }
    }

    unsigned int loadTexture(int textureIndx) {

        if (textureIndx == -1) return 0;

        unsigned int textureID;
        if (textureIndx < 0 || textureIndx >= (int)tinygltf_model->textures.size()) {
            std::cout << "index textures is out of bound\n";
            std::cout << "textureIndx: " << textureIndx << "\n";
            return 0;
        }
        tinygltf::Texture& t = tinygltf_model->textures[textureIndx];
        if (t.source < 0 || t.source >= (int)tinygltf_model->images.size()) {
            std::cout << "index source image is out of bound\n";
            std::cout << "textureIndx: " << t.source << "\n";
            return 0;
        }
        tinygltf::Image& image = tinygltf_model->images[t.source];
        tinygltf::Sampler sampler;
        if (t.sampler < 0 || t.sampler >= (int)tinygltf_model->samplers.size()) {
            std::cout << "index sampler is out of bound\n";
            std::cout << "textureIndx: " << t.sampler << "\n";
            sampler.wrapS = GL_REPEAT;
            sampler.wrapT = GL_REPEAT;
            sampler.minFilter = GL_LINEAR;
            sampler.magFilter = GL_LINEAR;
        }
        else {
            sampler = tinygltf_model->samplers[t.sampler];
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

    void getTextureDimension(Materials& pMaterial, int textureIndex) {
        if (textureIndex < 0 || textureIndex >= (int)tinygltf_model->textures.size()) {
            return;
        }
        tinygltf::Texture& t = tinygltf_model->textures[textureIndex];
        if (t.source < 0 || t.source >= (int)tinygltf_model->images.size()) {
            return;
        }
        tinygltf::Image& image = tinygltf_model->images[t.source];

        pMaterial.width = image.width;
        pMaterial.height = image.height;
    }

    void bindMaterial(gltf::Model& model, int materialIndx) {
        if (materialIndx < 0 || materialIndx >= (int)tinygltf_model->materials.size()) {
            std::cout << "index material is out of bound\n";
            std::cout << "textureIndx: " << materialIndx << "\n";
            return;
        }

        Materials& currMaterial = model.materials[materialIndx];
        if (currMaterial.useMaterial == true) {
            return;
        }
        tinygltf::Material& t_material = tinygltf_model->materials[materialIndx];

        std::vector<double>& colorFactor = t_material.pbrMetallicRoughness.baseColorFactor;
        for (int i = 0; i < colorFactor.size(); i++) {
            currMaterial.baseColor[i] = colorFactor[i];
        }
        currMaterial.metallicFactor = t_material.pbrMetallicRoughness.metallicFactor;
        currMaterial.roughnessFactor = t_material.pbrMetallicRoughness.roughnessFactor;

        getTextureDimension(currMaterial, t_material.pbrMetallicRoughness.baseColorTexture.index);
        currMaterial.albedoMap = loadTexture(t_material.pbrMetallicRoughness.baseColorTexture.index);
        currMaterial.normalMap = loadTexture(t_material.normalTexture.index);
        currMaterial.roughnessMap = loadTexture(t_material.pbrMetallicRoughness.metallicRoughnessTexture.index);
        currMaterial.emissiveMap = loadTexture(t_material.emissiveTexture.index);
        currMaterial.occlusionMap = loadTexture(t_material.occlusionTexture.index);
        currMaterial.useMaterial = true;

        currMaterial.Map["roughnessMap"] = currMaterial.roughnessMap;
        currMaterial.Map["metallicMap"] = currMaterial.metallicMap;
        currMaterial.Map["occlusionMap"] = currMaterial.occlusionMap;
    }

    void bindAttributeIndex(tinygltf::Primitive& prim) {

        for (auto& attr : prim.attributes) {
            tinygltf::Accessor& accessor = tinygltf_model->accessors[attr.second];
            if (accessor.bufferView < 0 || accessor.bufferView >= (int)tinygltf_model->bufferViews.size()) {
                std::cout << "index bufferView is out of bound\n";
                std::cout << "indx: " << accessor.bufferView << "\n";
                continue;
            }
            tinygltf::BufferView& bufferView = tinygltf_model->bufferViews[accessor.bufferView];
            if (bufferView.buffer < 0 || bufferView.buffer >= (int)tinygltf_model->buffers.size()) {
                std::cout << "index buffer is out of bound\n";
                std::cout << "indx: " << bufferView.buffer << "\n";
                continue;
            }
            tinygltf::Buffer& buffer = tinygltf_model->buffers[bufferView.buffer];

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

            unsigned int offsetofData = bufferView.byteOffset + accessor.byteOffset;
            unsigned int stride = accessor.ByteStride(bufferView);
            unsigned int lengthOfData = accessor.count * stride;

            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, lengthOfData, &buffer.data.at(offsetofData), GL_STATIC_DRAW);
            if (vaa->first == "JOINTS_0") {
                glVertexAttribIPointer(vaa->second, accessor.type, accessor.componentType,
                    stride, (void*)(0));
            }
            else {
                glVertexAttribPointer(vaa->second, accessor.type, accessor.componentType,
                    accessor.normalized ? GL_TRUE : GL_FALSE, stride, (void*)(0));
            }
            glEnableVertexAttribArray(vaa->second);
        }
    }

    void bindMesh(gltf::Model& model, int nodeIndx, int meshIndx) {
        if (meshIndx < 0 || meshIndx >= (int)tinygltf_model->meshes.size()) {
            return;
        }

        tinygltf::Mesh& mesh = tinygltf_model->meshes[meshIndx];
        //printf("---------------------------------\n");
        //printf("Mesh ke - %d\n", meshIndx);
        //printf("mesh name: %s\n", mesh.name.c_str());

        for (int i = 0; i < mesh.primitives.size(); i++) {

            tinygltf::Primitive& prim = mesh.primitives[i];
            if (prim.indices < 0 || prim.indices >= (int)tinygltf_model->accessors.size()) {
                std::cout << "index indices of accessor is out of bound\n";
                std::cout << "indx: " << prim.indices << "\n";
                return;
            }
            tinygltf::Accessor& accessor = tinygltf_model->accessors[prim.indices];
            if (accessor.bufferView < 0 || accessor.bufferView >= (int)tinygltf_model->bufferViews.size()) {
                std::cout << "index bufferView is out of bound\n";
                std::cout << "indx: " << accessor.bufferView << "\n";
                return;
            }
            tinygltf::BufferView& bufferView = tinygltf_model->bufferViews[accessor.bufferView];
            if (bufferView.buffer < 0 || bufferView.buffer >= (int)tinygltf_model->buffers.size()) {
                std::cout << "index buffer is out of bound\n";
                std::cout << "indx: " << bufferView.buffer << "\n";
                return;
            }
            tinygltf::Buffer& buffer = tinygltf_model->buffers[bufferView.buffer];

            //printf("primitive[%d]\n", i);
            //printf("prim.indices = %d\n", prim.indices);
            //printf("accessor.bufferView = %d\n", accessor.bufferView);
            //printf("bufferView.buffer = %d\n", bufferView.buffer);

            unsigned int vao;
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            unsigned int ebo;
            glGenBuffers(1, &ebo);
            unsigned int offsetofData = bufferView.byteOffset + accessor.byteOffset;
            unsigned int stride = accessor.ByteStride(bufferView);
            unsigned int lengthOfData = accessor.count * stride;

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, lengthOfData, &buffer.data.at(offsetofData), GL_STATIC_DRAW);

            bindAttributeIndex(prim);
            bindMaterial(model, prim.material);

            gltf::Model::AttributeObject attr(vao, ebo, prim.material);
            attr.setDrawMode(prim.mode, accessor.count, accessor.componentType);
            model.attributes.push_back(attr);
            model.nodes[nodeIndx].meshIndices.push_back(model.attributes.size() - 1);

            glBindVertexArray(0);
        }
    }

    void bindSkin(gltf::Model& model, int nodeIndx, int skinIndx) {

        if (skinIndx == -1) return;

        tinygltf::Skin& skin = tinygltf_model->skins[skinIndx];
        //printf("skin name ---- %s\n", skin.name.c_str());
        if (skin.inverseBindMatrices < 0 || skin.inverseBindMatrices >= tinygltf_model->accessors.size()) {
            printf("Skin Inverse Bind Matrices is not found!\n");
            return;
        }
        tinygltf::Accessor& accessor = tinygltf_model->accessors[skin.inverseBindMatrices];
        if (accessor.bufferView < 0 || accessor.bufferView >= tinygltf_model->bufferViews.size()) {
            printf("Skin accessor bufferView is not found!\n");
            return;
        }
        if (accessor.type != 36) {
            printf("error Skin Type data!\naccessor.type = %d\n", accessor.type);
            return;
        }
        tinygltf::BufferView& bufferView = tinygltf_model->bufferViews[accessor.bufferView];
        if (bufferView.buffer < 0 || bufferView.buffer >= tinygltf_model->buffers.size()) {
            printf("Skin buffer data is not found!\n");
            return;
        }
        tinygltf::Buffer& buffer = tinygltf_model->buffers[bufferView.buffer];

        unsigned int offsetofData = bufferView.byteOffset + accessor.byteOffset;
        unsigned int stride = accessor.ByteStride(bufferView);
        unsigned int lengthOfData = accessor.count * stride;

        int joint_n = tinygltf_model->skins[skinIndx].joints.size();
        for (size_t j = 0; j < joint_n; ++j) {
            glm::mat4 mt = glm::make_mat4((float*)(buffer.data.data() + offsetofData + stride * j));
            int joint = tinygltf_model->skins[skinIndx].joints[j];
            model.nodes[joint].transform.inverseMatrix = mt;
            //tinygltf::Node& nodeTemp = model.nodes[joint];
            //printf("joint %d : %s\n", joint, nodeTemp.name.c_str());
            //std::cout << to_string(mt) << "\n";
        }
        model.skeletals[skinIndx].first = tinygltf_model->skins[skinIndx].skeleton;
        model.skeletals[skinIndx].second = tinygltf_model->skins[skinIndx].joints;
    }

    void bindNodes(gltf::Model& model, int nodeIndx, int nodeParent) {
        if (nodeIndx < 0 || nodeIndx >= (int)tinygltf_model->nodes.size()) {
            std::cout << "index node is out of bound\n";
            std::cout << "indx: " << nodeIndx << "\n";
            return;
        }

        tinygltf::Node& t_node = tinygltf_model->nodes[nodeIndx];
        //printf("node[%d] name: %s\n", nodeIndx, t_node.name.c_str());

        Transformation& nodeTransform = model.nodes[nodeIndx].transform;
        setNodeTransform(t_node, nodeTransform);
        nodeTransform.localTransform();
        if (nodeParent != -1) {
            model.nodes[nodeParent].childNode.push_back(nodeIndx);
            nodeTransform.matrix = nodeTransform.matrix * model.nodes[nodeParent].transform.matrix;
            //inverseMatrices[indx] = glm::inverse(modelMatrices[indx]);
        }
        if (t_node.mesh != -1) bindMesh(model, nodeIndx, t_node.mesh);
        if (t_node.skin != -1) bindSkin(model, nodeIndx, t_node.skin);

        for (int i = 0; i < t_node.children.size(); i++) {
            bindNodes(model, t_node.children[i], nodeIndx);
        }
    }

    void loadScene(gltf::Model& model) {

        model.nodes.resize(tinygltf_model->nodes.size(), Model::NodeObject());
        model.materials.resize(tinygltf_model->materials.size(), Materials());
        model.skeletals.resize(tinygltf_model->skins.size(), { -1, std::vector<int>() });
        int defaultScene = tinygltf_model->defaultScene;
        tinygltf::Scene& scene = tinygltf_model->scenes[defaultScene];
        for (int node : scene.nodes) {
            model.rootNode.push_back(node);
            bindNodes(model, node, -1);
        }
    }

    void loadAnimation(gltf::Model& model) {
        bool ret = false;

        int current_animation = 0;
        model.animator.animations.resize(tinygltf_model->animations.size());
        for (tinygltf::Animation& animation : tinygltf_model->animations) {
            //printf("%d: %s\n", current_animation, animation.name.c_str());
            int sampler_length = animation.samplers.size();
            int channel_length = animation.channels.size();
            model.animator.animations[current_animation].name = animation.name;
            for (tinygltf::AnimationChannel& channel : animation.channels) {

                if (current_animation > 0) break;

                int indxSampler = channel.sampler;
                int targetNode = channel.target_node;

                //printf("target node animation: %d\n", targetNode);

                std::string targetPath = channel.target_path;
                if (indxSampler < 0 || indxSampler >= sampler_length) {
                    printf("E| animation index sampler is not found!\n");
                    printf("E| indxSampler: %d\n", indxSampler);
                    break;
                }
                tinygltf::AnimationSampler& sampler = animation.samplers[indxSampler];
                tinygltf::Accessor& accessorInput = tinygltf_model->accessors[sampler.input];
                tinygltf::BufferView& bufferViewInput = tinygltf_model->bufferViews[accessorInput.bufferView];
                tinygltf::Buffer& bufferInput = tinygltf_model->buffers[bufferViewInput.buffer];

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
                //printf("timestamp: ");
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
                //printf("count = %d,, firstTime = %f - lastTimeStamp = %f\n", inputData.size(), inputData[0], inputData[inputData.size() - 1]);
                //printf("\n");
                tinygltf::Accessor& accessorOutput = tinygltf_model->accessors[sampler.output];
                tinygltf::BufferView& bufferViewOutput = tinygltf_model->bufferViews[accessorOutput.bufferView];
                tinygltf::Buffer& bufferOutput = tinygltf_model->buffers[bufferViewOutput.buffer];

                offsetofData = bufferViewOutput.byteOffset + accessorOutput.byteOffset;
                stride = accessorOutput.ByteStride(bufferViewOutput);
                lengthOfData = accessorOutput.count * stride;

                std::vector<glm::vec4> outputData;
                for (int j = 0; j < accessorOutput.count; j++) {
                    glm::vec4 temp = glm::make_vec4((float*)(bufferOutput.data.data() + offsetofData + stride * j));
                    outputData.push_back(temp);
                }
                model.animator.animations[current_animation].addKeyframe(inputData, outputData, targetNode, sampler.interpolation, targetPath);
            }
            current_animation++;
        }
    }

    bool gltfLoadModel(const char* filename) {
        bool ret = false;
        std::string err;
        std::string warn;
        tinygltf::TinyGLTF loader;

        if (tinygltf_model != nullptr) {
            delete tinygltf_model;
            tinygltf_model = nullptr;
        }

        if (tinygltf_model == nullptr) {
            tinygltf_model = new tinygltf::Model();
        }

        if (strstr(filename, ".gltf") != NULL) {
            ret = loader.LoadASCIIFromFile(tinygltf_model, &err, &warn, filename);
        }

        if (strstr(filename, ".glb") != NULL) {
            ret = loader.LoadBinaryFromFile(tinygltf_model, &err, &warn, filename); // for binary glTF(.glb)
        }

        if (!warn.empty()) {
            printf("Warn: %s\n", warn.c_str());
        }

        if (!err.empty()) {
            printf("Err: %s\n", err.c_str());
        }

        if (!ret) {
            printf("Failed to parse glTF\n");
            return false;
        }
        printf("Loaded glTF: %s\n", filename);
        return ret;
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

    static float HexToFloat(unsigned char temp[]) {
        uint32_t x = temp[3];
        for (int i = 2; i >= 0; i--) x = (x << 8) | temp[i];
        static_assert(sizeof(float) == sizeof(uint32_t), "Float and uint32_t size dont match. Check another int type");

        float f{};
        memcpy(&f, &x, sizeof(x));

        return f;
    }

}

#endif