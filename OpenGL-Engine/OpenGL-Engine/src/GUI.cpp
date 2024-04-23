#include "GUI.h"

namespace GUI {

    static TerrainParam _tp(4, 27.9f, 4, 0.5f, 2.0f, {0.0f, 0.0f}, 5.0f);
    static GrassParam _gp();

    void initialize(GLFWwindow* w) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

        ImGui_ImplGlfw_InitForOpenGL(w, true);
        ImGui_ImplOpenGL3_Init();
    }

    void GUIFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        //ImGui::ShowDemoWindow(); // Show demo window! :)

        ImGui::Begin("Demo window");
        ImGui::Button("Hello!");
    }

    void modelTransform(glm::vec3& pos, glm::vec3& rot, float& angle, glm::vec3& scale) {
        float modelPos[3] = { pos.x, pos.y, pos.z };
        ImGui::DragFloat3("position", modelPos, 0.005f);
        pos[0] = modelPos[0];
        pos[1] = modelPos[1];
        pos[2] = modelPos[2];

        ImGui::SliderAngle("angle", &angle);
        float modelRot[3] = { rot.x, rot.y, rot.z };
        ImGui::SliderFloat3("rotation", modelRot, 0.000, 1.000);
        rot[0] = modelRot[0];
        rot[1] = modelRot[1];
        rot[2] = modelRot[2];

        float modelScale[3] = { scale.x, scale.y, scale.z };
        ImGui::DragFloat3("scale", modelScale);
        scale[0] = modelScale[0];
        scale[1] = modelScale[1];
        scale[2] = modelScale[2];
    }

    // later;
    void modelAnimation(int animation, int n) {

        //const char* animationName[] = { "Animation 0", "no animation" };

        //static int currentAnimation = animation;
        //ImGui::Combo("Animation", &currentAnimation, animationName, 2);
        //if (currentAnimation != animation) {
        //    if (currentAnimation == 0) {
        //        //carafe.animator.doAnimation(currentAnimation);
        //    }
        //    else {
        //        //carafe.animator.doAnimation(-1);
        //    }
        //}
    }



    bool proceduralTerrainParam(int& seed, float& scale, int& octaves, float& persistence, float& lacunarity, glm::vec2& offset, float& heightMultiplier) {

        bool ret = false;

        ImGui::DragInt("seed", &_tp.m_Seed);
        ImGui::DragFloat("scale", &_tp.m_Scale, 0.01f, 0.01f);
        ImGui::DragInt("octaves", &_tp.m_Octaves, 1, 1, 16);
        ImGui::DragFloat("persistence", &_tp.m_Persistence, 0.01f, 0.01f);
        ImGui::DragFloat("lacunarity", &_tp.m_Lacunarity, 0.01f, 0.01f);
        ImGui::DragFloat2("offset", _tp.m_Offset, 0.01f);
        ImGui::DragFloat("elevation", &_tp.m_Amplitude, 0.1f, 0.01f);

        if (_tp.m_Seed != seed || _tp.m_Scale != scale || _tp.m_Octaves != octaves || _tp.m_Persistence != persistence || _tp.m_Lacunarity != lacunarity ||
            _tp.m_Offset[0] != offset.x || _tp.m_Offset[1] != offset.y || _tp.m_Amplitude != heightMultiplier) {
            ret = true;
        }

        seed = _tp.m_Seed;
        scale = _tp.m_Scale;
        octaves = _tp.m_Octaves;
        persistence = _tp.m_Persistence;
        lacunarity = _tp.m_Lacunarity;
        offset.x = _tp.m_Offset[0]; offset.y = _tp.m_Offset[1];
        heightMultiplier = _tp.m_Amplitude;

        return ret;
    }

    void grassParam(float& frequency, float& amplitude, float& scale, float& drop) {
        ImGui::DragFloat("frequency", &frequency, 0.01f, 0.01f);
        ImGui::DragFloat("amplitude", &amplitude, 0.01f, 0.01f);
        ImGui::DragFloat("_Scale", &scale, 0.01f, 0.01f);
        ImGui::DragFloat("_Droop", &drop, 0.01f, 0.01f);
    }

    void renderUI() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    }

    void shutDown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}
