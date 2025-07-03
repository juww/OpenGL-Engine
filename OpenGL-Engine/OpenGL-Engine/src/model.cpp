#include "model.h"
#include "loadModel.h"

namespace gltf {

    Model::Model() {
        shader = nullptr;

        pos = glm::vec3(0.0f, 0.0f, 0.0f);
        rot = glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f));
        scale = glm::vec3(1.0f);
        angle = 0.0f;
    }

    Model::~Model() {
    }

    void Model::setPosition(glm::vec3 pPos) {
        pos = pPos;
    }

    void Model::setRotation(glm::vec3 pRot, float pAngle) {
        rot = pRot;
        angle = pAngle;
    }

    void Model::setScale(glm::vec3 pScale) {
        scale = pScale;
    }

    void Model::drawMesh(int meshIndex) {

        AttributeObject& attr = attributes[meshIndex];

        glBindVertexArray(attr.vao);

        if (attr.materialIndx != -1) {
            Materials& material = materials[attr.materialIndx];

            shader->setBool("useAlbedoMapping", material.albedoMap != 0);
            shader->setInt("albedoMap", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, material.albedoMap);

            shader->setBool("useNormalMapping", material.normalMap != 0);
            shader->setInt("normalMap", 1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, material.normalMap);

            shader->setBool("useRoughnessMapping", material.roughnessMap != 0);
            shader->setInt("roughnessMap", 2);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, material.roughnessMap);
    
            shader->setBool("useMetallicMapping", material.metallicMap != 0);
            shader->setInt("metallicMap", 3);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, material.metallicMap);

            shader->setBool("useOcclusionMapping", material.occlusionMap != 0);
            shader->setInt("occlusionMap", 4);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, material.occlusionMap);

            shader->setBool("useEmissiveMapping", material.emissiveMap != 0);
            shader->setInt("emissiveMap", 5);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, material.emissiveMap);

            shader->setBool("useMROMapping", material.metallicRoughnessOcclusionTexture != 0);
            shader->setInt("MROMap", 9);
            glActiveTexture(GL_TEXTURE9);
            glBindTexture(GL_TEXTURE_2D, material.metallicRoughnessOcclusionTexture);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, attr.ebo);
        glDrawElements(attr.drawMode, attr.count, attr.type, (void*)(0));

        turnOffAllTexture();
        glBindVertexArray(0);
    }

    void Model::drawNodes(int nodeIndx) {
        if (nodeIndx < 0 || nodeIndx >= (int)nodes.size()) {
            return;
        }
        NodeObject& node = nodes[nodeIndx];
        //glm::mat4 mat(1.0f);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
        //model = glm::rotate(model, (float)glfwGetTime(), { 1.0f,0.0f,0.0f });
        glm::mat4 R = glm::rotate(glm::mat4(1.0f), angle, rot);
        //scale = glm::vec3(0.01f);
        //std::cout << glm::to_string(mat) << "\n";
        glm::mat4 T = glm::translate(glm::mat4(1.0f), pos);

        glm::mat4 mat = T * R * S;
        mat = mat * nodes[nodeIndx].transform.matrix;
        shader->setMat4("model", mat);
        
        for (auto& meshIndex : nodes[nodeIndx].meshIndices) {
            drawMesh(meshIndex);
        }

        for (int i = 0; i < node.childNode.size(); i++) {
            drawNodes(node.childNode[i]);
        }
    }

    void Model::draw() {

        shader->use();
        //glEnable(GL_CULL_FACE);
        for (int node : rootNode) {
            drawNodes(node);
        }
        //glDisable(GL_CULL_FACE);
    }

    void Model::updateSkeletalNode(int nodeIndx, int parent) {
        if (nodeIndx < 0) {
            return;
        }

        std::map<int, Transformation>::iterator itr = animator.currentPose.find(nodeIndx);
        if (itr == animator.currentPose.end()) {
            animator.currentPose.insert({ nodeIndx, animator.nodeDefaultTransform[nodeIndx] });
            itr = animator.currentPose.find(nodeIndx);
        }
        Transformation& transform = animator.currentPose[nodeIndx];
        transform.matrix = glm::mat4(1.0f);

        transform.localTransform();
        if (parent != -1) {
            Transformation& par = animator.currentPose.find(parent)->second;
            transform.matrix = par.matrix * transform.matrix;
        }

        for (unsigned int child : nodes[nodeIndx].childNode) {
            updateSkeletalNode(child, nodeIndx);
        }
    }

    void Model::update(float deltaTime) {

        shader->use();
        if (!animator.update(deltaTime)) {
            playAnimation = false;
            shader->setInt("hasBone", 0);
            return;
        }
        shader->setInt("hasBone", 1);
        playAnimation = true;
        for (auto &skeletal : skeletals) {
            int root = skeletal.first;
            updateSkeletalNode(root, -1);
            for (int i = 0; i < skeletal.second.size(); i++) {
                int nodeIndx = skeletal.second[i];
                std::map<int, Transformation>::iterator itr = animator.currentPose.find(nodeIndx);
                Transformation &animationTranform = itr->second;
                
                animationTranform.matrix = animationTranform.matrix * nodes[nodeIndx].transform.inverseMatrix;
                shader->setMat4("boneTransform[" + std::to_string(i) + "]", animationTranform.matrix);
            }
        }
    }

    void Model::setUniforms(const glm::mat4& projection, const glm::mat4& view, glm::vec3& cameraPos,
        const float& _time, std::map<std::string, unsigned int>& mappers, std::vector<glm::vec3> lightPos, GUI::PBRParam& pbr) {
        shader->use();

        shader->setMat4("projection", projection);
        shader->setMat4("view", view);

        for (int i = 0; i < lightPos.size(); i++) {
            shader->setVec3("lightPosition[" + std::to_string(i) + "]", lightPos[i]);
        }
        //shader->setVec3("lightPos", lightPos);
        shader->setVec3("viewPos", cameraPos);

        shader->setVec4("baseColor", pbr.m_BaseColor);
        shader->setFloat("roughnessFactor", pbr.m_RoughnessFactor);
        shader->setFloat("subSurface", pbr.m_SubSurface);
        shader->setFloat("metallicFactor", pbr.m_MetallicFactor);

        shader->setFloat("_Specular", pbr.m_Specular);
        shader->setFloat("_SpecularTint", pbr.m_SpecularTint);
        shader->setFloat("_Sheen", pbr.m_Sheen);
        shader->setFloat("_SheenTint", pbr.m_SheenTint);
        shader->setFloat("_Anisotropic", pbr.m_Anisotropic);
        shader->setFloat("_ClearCoatGloss", pbr.m_ClearCoatGloss);
        shader->setFloat("_ClearCoat", pbr.m_ClearCoat);

        shader->setInt("shadowMap", 10);
        shader->setInt("shadowCubeMap", 11);
        shader->setInt("useShadowMapping", 0);

        shader->setInt("irradianceMap", 6);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mappers["irradianceMap"]);

        shader->setInt("preFilterMap", 7);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mappers["preFilterMap"]);

        shader->setInt("brdfLUTTexture", 8);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, mappers["brdfLUTTexture"]);
    }

    bool Model::loadModel(const char* filename) {
        bool ret = gltfLoadModel(filename);
        if (!ret) {
            return false;
        }

        loadScene(*this);
        loadAnimation(*this);
    }

    void Model::setShader(std::string filename) {
        std::string vs = filename + ".vs";
        std::string fs = filename + ".fs";
        shader = new Shader(vs, fs, "");
    }

    void Model::setShader(Shader* p_shader) {
        shader = p_shader;
    }

    void Model::turnOffAllTexture() {
        for (int i = 0; i < 6; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Model::AttributeObject::AttributeObject() {
        vao = 0;
        ebo = 0;
        materialIndx = -1;
        drawMode = 0;
        count = 0;
        type = 0;
    }

    Model::AttributeObject::AttributeObject(unsigned int p_vao, unsigned int p_ebo, unsigned int p_material) {
        vao = p_vao;
        ebo = p_ebo;
        materialIndx = p_material;
        drawMode = 0;
        count = 0;
        type = 0;
    }

    void Model::AttributeObject::setDrawMode(int p_mode, int p_count, int p_type) {
        drawMode = p_mode;
        count = p_count;
        type = p_type;
    }

    Model::NodeObject::NodeObject() {
        transform = Transformation();
        childNode.clear();
        meshIndices.clear();
    }
}