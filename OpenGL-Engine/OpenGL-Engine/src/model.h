#ifndef MODEL_H
#define MODEL_H

#include <TinyGLTF/tiny_gltf.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

#include "material.h"
#include "transformation.h"
#include "animator.h"
#include "GUI.h"

namespace gltf {

    class Model {
    public:
        glm::vec3 pos;
        glm::vec3 rot;
        glm::vec3 scale;
        float angle;
        std::vector<int> rootNode;

        struct AttributeObject {
            unsigned int vao, ebo;
            unsigned int materialIndx;
            int drawMode, count, type;
            AttributeObject();
            AttributeObject(unsigned int p_vao, unsigned int p_ebo, unsigned int p_material);
            void setDrawMode(int p_mode, int p_count, int p_type);
        };
        struct NodeObject {
        public:
            std::string name;
            Transformation transform;
            std::vector<unsigned int> childNode;
            std::vector<unsigned int> meshIndices;
            NodeObject();
        };
        std::vector<AttributeObject> attributes;
        std::vector<Materials> materials;
        std::vector<NodeObject> nodes;
        std::vector<std::pair<int, std::vector<int> > > skeletals;
        bool playAnimation;
        Animator animator;
        Shader* shader;

        Model();
        ~Model();

        void setPosition(glm::vec3 pPos);
        void setRotation(glm::vec3 pRot, float pAngle);
        void setScale(glm::vec3 pScale);

        void drawMesh(int indx);
        void drawNodes(int indx);
        void draw();
        void update(float deltaTime);
        void updateSkeletalNode(int nodeIndx, int parent);
        bool loadModel(const char* filename);
        void setUniforms(const glm::mat4& projection, const glm::mat4& view, glm::vec3& cameraPos,
            const float& _time, std::map<std::string, unsigned int>& mappers, std::vector<glm::vec3> lightPos, GUI::PBRParam& pbr);
        void setShader(std::string filename);
        void setShader(Shader *p_shader);
        void turnOffAllTexture();
    };
}

#endif // !MODEL_H
