#include "GUI.h"
#include "animator.h"

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
        //ImGui::ShowDemoWindow();
    }

    void PBRWindow(PBRParam& pbr) {
        ImGui::Begin("PBR");

        float baseColor[4] = { pbr.m_BaseColor.r, pbr.m_BaseColor.g, pbr.m_BaseColor.b, pbr.m_BaseColor.a};
        ImGui::ColorEdit4("Color", baseColor);
        ImGui::DragFloat("roughnessFactor", &pbr.m_RoughnessFactor, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("subSurface", &pbr.m_SubSurface, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("metallicFactor", &pbr.m_MetallicFactor, 0.001f, 0.0f, 1.0f);

        ImGui::DragFloat("Specular", &pbr.m_Specular, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("SpecularTint", &pbr.m_SpecularTint, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("Sheen", &pbr.m_Sheen, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("SheenTint", &pbr.m_SheenTint, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("Anisotropic", &pbr.m_Anisotropic, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("ClearCoatGloss", &pbr.m_ClearCoatGloss, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("ClearCoat", &pbr.m_ClearCoat, 0.001f, 0.0f, 1.0f);

        ImGui::DragFloat("heightScale", &pbr.m_HeightScale, 0.001f, 0.0f, 1.0f);

        pbr.m_BaseColor = glm::vec4(baseColor[0], baseColor[1], baseColor[2], baseColor[3]);

        ImGui::End();
    }

    void modelTransform(std::string name, glm::vec3& pos, glm::vec3& rot, float& angle, glm::vec3& scale) {

        ImGui::Begin(name.c_str());

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

        ImGui::End();
    }

    // later;
    void modelAnimation(std::string name, Animator &animator) {

        ImGui::Begin(name.c_str());
        ImGui::SeparatorText("animation");
        int n_animation = animator.animations.size();
        if (ImGui::BeginCombo("animation", animator.animations[0].name.c_str(), 0)) {
            for (int i = 0; i < n_animation; i++) {
                bool isSelected = (animator.currentAnimation == i);
                if (ImGui::Selectable(animator.animations[i].name.c_str(), isSelected))
                    animator.currentAnimation = i;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::Text("length : %f", animator.animations[animator.currentAnimation].length);
        ImGui::Text("count : %d", animator.animations[animator.currentAnimation].count);
        ImGui::Text("timestamp size : %d", animator.animations[animator.currentAnimation].timestamp.size());
        ImGui::Text("keyframe size : %d", animator.animations[animator.currentAnimation].keyframes.size());
        if (ImGui::Button("previous")) {
            if (animator.currentKeyframe > 0) {
                animator.currentKeyframe--;
            }
            animator.nextKeyframe = animator.currentKeyframe + 1;
            animator.animationTime = animator.animations[animator.currentAnimation].
                keyframes[animator.IndexKeyframes[animator.currentKeyframe]].Timestamp;
        }
        ImGui::SameLine();
        if (ImGui::Button("play")) {
            animator.playAnimation = !(animator.playAnimation);
        }
        ImGui::SameLine();
        if (ImGui::Button("next")) {
            if (animator.nextKeyframe < animator.IndexKeyframes.size() - 1) {
                animator.nextKeyframe++;
            }
            animator.currentKeyframe = animator.nextKeyframe - 1;
            animator.animationTime = animator.animations[animator.currentAnimation].
                keyframes[animator.IndexKeyframes[animator.currentKeyframe]].Timestamp;
        }

        float progress = animator.animationTime, lengthTime = animator.animations[animator.currentAnimation].length;
        char buf[32];
        sprintf(buf, "%.4f/%.4f", progress, lengthTime);
        ImGui::ProgressBar(progress/lengthTime, ImVec2(0.f, 0.f), buf);

        bool itemHighlight = false;
        int itemSelected = 0, itemHighlighted = -1;
        int indx = 0;
        if (ImGui::BeginListBox("timestamp list")) {
            for (auto& time : animator.animations[animator.currentAnimation].timestamp) {
                bool isSelected = (itemSelected == indx);
                std::string label = std::to_string(time.first) + " - " + std::to_string(time.second);
                if (ImGui::Selectable(label.c_str(), isSelected))
                    itemSelected = indx;

                if (itemHighlight && ImGui::IsItemHovered())
                    itemHighlighted = indx;

                if (isSelected)
                    ImGui::SetItemDefaultFocus();

                indx++;
            }
            ImGui::EndListBox();
        }

        if (ImGui::BeginListBox("keyframe time list")) {
            for (auto& keyframe : animator.animations[animator.currentAnimation].keyframes) {
                bool isSelected = (itemSelected == indx);
                std::string label = std::to_string(keyframe.Timestamp);
                if (ImGui::Selectable(label.c_str(), isSelected))
                    itemSelected = indx;

                if (itemHighlight && ImGui::IsItemHovered())
                    itemHighlighted = indx;

                if (isSelected)
                    ImGui::SetItemDefaultFocus();

                indx++;
            }
            ImGui::EndListBox();
        }

        ImGui::PushID("##HorizontalScrolling");
        ImGuiStyle& style = ImGui::GetStyle();
        static int track_item = 3;
        static bool enable_track = true;
        auto &nodes = animator.nodeAnimation[0];
        for (int i = 0; i < nodes.size(); i++) {
            float child_height = ImGui::GetTextLineHeight() + style.ScrollbarSize + style.WindowPadding.y * 6.0f;
            ImGuiWindowFlags child_flags = ImGuiWindowFlags_HorizontalScrollbar;
            ImGuiID child_id = ImGui::GetID((void*)(intptr_t)i);
            bool child_is_visible = ImGui::BeginChild(child_id, ImVec2(-100, child_height), true, child_flags);
            if (child_is_visible) {
                for (int j = 0; j < nodes[i].translate.size(); j++){
                    if (j == 0) {
                        ImGui::Text("translation:\t");
                    }
                    ImGui::SameLine();
                    if (enable_track && j == track_item) {
                        ImGui::TextColored(ImVec4(1, 1, 0, 1), "%.3f", nodes[i].translate[j].first);
                    } else {
                        ImGui::Text("%.3f", nodes[i].translate[j].first);
                    }
                }
                for (int j = 0; j < nodes[i].rotate.size(); j++) {
                    if (j == 0) {
                        ImGui::Text("rotation:\t");
                    }
                    ImGui::SameLine();
                    if (enable_track && j == track_item) {
                        ImGui::TextColored(ImVec4(1, 1, 0, 1), "%.3f", nodes[i].rotate[j].first);
                    } else {
                        ImGui::Text("%.3f", nodes[i].rotate[j].first);
                    }
                }
                for (int j = 0; j < nodes[i].scale.size(); j++) {
                    if (j == 0) {
                        ImGui::Text("scale:\t");
                    }
                    ImGui::SameLine();
                    if (enable_track && j == track_item) {
                        ImGui::TextColored(ImVec4(1, 1, 0, 1), "%.3f", nodes[i].scale[j].first);
                    }
                    else {
                        ImGui::Text("%.3f", nodes[i].scale[j].first);
                    }
                }
            }
            float scroll_x = ImGui::GetScrollX();
            float scroll_max_x = ImGui::GetScrollMaxX();
            ImGui::EndChild();
            ImGui::SameLine();
            
            ImGui::Text("node %d\n%.0f/%.0f", i, scroll_x, scroll_max_x);
            ImGui::Spacing();
        }
        ImGui::PopID();

        ImGui::End();
    }

    void waterFFTParam(WaterFFTParam &waterFFTParams) {
        ImGui::Begin("water FFT");

        ImGui::SeparatorText("water FFT parameter");
        ImGui::DragInt("seed", &waterFFTParams.waterUniform.seed);
        ImGui::DragInt("arrayTextureSize", &waterFFTParams.waterUniform.arrayTextureSize, 1, 1, 4);
        ImGui::DragFloat3("lightDirection", waterFFTParams.waterUniform.lightDirection);
        ImGui::DragFloat("lowCutoff", &waterFFTParams.waterUniform.lowCutoff, 0.0001f, 0.0f, 1.0f);
        ImGui::DragFloat("highCutoff", &waterFFTParams.waterUniform.highCutoff, 1.0f, 1.0f, 9000.0f);
        ImGui::DragFloat("gravity", &waterFFTParams.waterUniform.gravity, 0.01f, 0.0f, 20.0f);
        ImGui::DragFloat("depth", &waterFFTParams.waterUniform.depth, 0.01f, 1.0f, 20.0f);
        ImGui::DragFloat("repeatTime", &waterFFTParams.waterUniform.repeatTime, 0.01f, 0.0f, 200.0f);
        ImGui::DragFloat("speed", &waterFFTParams.waterUniform.speed, 0.01f, 0.0f, 20.0f);
        ImGui::DragFloat2("lambda", waterFFTParams.waterUniform.lambda);
        ImGui::DragFloat("displacementDepthFalloff", &waterFFTParams.waterUniform.displacementDepthFalloff);
        ImGui::DragFloat("normalStrength", &waterFFTParams.waterUniform.normalStrength);
        ImGui::DragFloat("normalDepthFalloff", &waterFFTParams.waterUniform.normalDepthFalloff);

        ImGui::SeparatorText("spectrum parameter");
        for (int i = 0; i < 4; i++) {
            if (i == 0) ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            ImGui::PushID(i);
            if (ImGui::TreeNode("", "spectrum %d", i)) {
                ImGui::DragFloat("lengthScale", &waterFFTParams.spectrumParam[i].lengthScale);
                ImGui::DragFloat("tile", &waterFFTParams.spectrumParam[i].tile, 1.0f);
                ImGui::DragFloat("scale", &waterFFTParams.spectrumParam[i].scale, 0.001f, 0.0f, 5.0f);
                ImGui::DragFloat("windSpeed", &waterFFTParams.spectrumParam[i].windSpeed, 0.1f);
                ImGui::DragFloat("windDirection", &waterFFTParams.spectrumParam[i].windDirection, 0.0f, 0.0f, 360.0f);
                ImGui::DragFloat("fetch", &waterFFTParams.spectrumParam[i].fetch);
                ImGui::DragFloat("spreadBlend", &waterFFTParams.spectrumParam[i].spreadBlend, 0.001f, 0.0f, 1.0f);
                ImGui::DragFloat("swell", &waterFFTParams.spectrumParam[i].swell, 0.001f, 0.0f, 1.0f);
                ImGui::DragFloat("peakEnhancement", &waterFFTParams.spectrumParam[i].peakEnhancement, 0.1f);
                ImGui::DragFloat("shortWavesFade", &waterFFTParams.spectrumParam[i].shortWavesFade, 0.01f);
                ImGui::Checkbox("useSpectrum", &waterFFTParams.spectrumParam[i].useSpectrum);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        ImGui::SeparatorText("PBR water parameter");
        ImGui::DragFloat("roughness", &waterFFTParams.PBRWater.roughness, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("metallic", &waterFFTParams.PBRWater.metallic, 0.001f, 0.0f, 1.0f);
        ImGui::ColorEdit3("sunIrradiance", waterFFTParams.PBRWater.sunIrradiance);
        ImGui::ColorEdit3("scatterColor", waterFFTParams.PBRWater.scatterColor);
        ImGui::ColorEdit3("bubbleColor", waterFFTParams.PBRWater.bubbleColor);
        ImGui::DragFloat("heightModifier", &waterFFTParams.PBRWater.heightModifier, 0.01f, 0.0f, 20.0f);
        ImGui::DragFloat("foamRoughnessModifier", &waterFFTParams.PBRWater.foamRoughnessModifier, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("bubbleDensity", &waterFFTParams.PBRWater.bubbleDensity, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("wavePeakScatterStrength", &waterFFTParams.PBRWater.wavePeakScatterStrength, 0.01f, 0.0f, 20.0f);
        ImGui::DragFloat("scatterStrength", &waterFFTParams.PBRWater.scatterStrength, 0.01f, 0.0f, 20.0f);
        ImGui::DragFloat("scatterShadowStrength", &waterFFTParams.PBRWater.scatterShadowStrength, 0.01f, 0.0f, 20.0f);
        ImGui::DragFloat("environmentLightStrength", &waterFFTParams.PBRWater.environmentLightStrength, 0.01f, 0.0f, 2.0f);

        ImGui::SeparatorText("foam parameter");
        ImGui::ColorEdit3("foamColor", waterFFTParams.foamParam.foamColor);
        ImGui::DragFloat("foamBias", &waterFFTParams.foamParam.foamBias, 0.01f, -2.0f, 2.0f);
        ImGui::DragFloat("foamDecayRate", &waterFFTParams.foamParam.foamDecayRate, 0.01f);
        ImGui::DragFloat("foamAdd", &waterFFTParams.foamParam.foamAdd, 0.01f);
        ImGui::DragFloat("foamThreshold", &waterFFTParams.foamParam.foamThreshold, 0.01f, -10.0f, 10.0f);
        ImGui::DragFloat("foamSubtract0", &waterFFTParams.foamParam.foamSubtract[0], 0.001f, -2.0f, 2.0f);
        ImGui::DragFloat("foamSubtract1", &waterFFTParams.foamParam.foamSubtract[1], 0.001f, -2.0f, 2.0f);
        ImGui::DragFloat("foamSubtract2", &waterFFTParams.foamParam.foamSubtract[2], 0.001f, -2.0f, 2.0f);
        ImGui::DragFloat("foamSubtract3", &waterFFTParams.foamParam.foamSubtract[3], 0.001f, -2.0f, 2.0f);

        ImGui::End();
    }

    bool proceduralTerrainParam(int& seed, float& scale, int& octaves, float& persistence, float& lacunarity, glm::vec2& offset, float& heightMultiplier) {

        bool ret = false;
        ImGui::Begin("Terrain Parameter");

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

        ImGui::End();
        return ret;
    }

    void grassParam(GrassParam& gp) {
        ImGui::Begin("Grass Parameter");

        ImGui::DragFloat("frequency", &gp.m_Frequency, 0.01f, 0.01f);
        ImGui::DragFloat("amplitude", &gp.m_Amplitude, 0.01f, 0.01f);
        ImGui::DragFloat("_Scale", &gp.m_Scale, 0.01f, 0.01f);
        ImGui::DragFloat("_Droop", &gp.m_Drop, 0.01f, 0.01f);

        ImGui::End();
    }

    void waterParam(WaterParam& wp) {
        ImGui::Begin("Water Parameter");

        ImGui::DragFloat("waterAmplitude", &wp.m_Amplitude, 0.01f, 0.0f,1.0f);
        ImGui::DragFloat("waterFrequency", &wp.m_Frequency, 0.01f, 1.0f);
        ImGui::DragFloat("waterSpeed", &wp.m_Speed, 0.01f, 0.01f);
        ImGui::DragInt("WaterWaveCount", &wp.m_WaveCount);

        ImGui::End();
    }

    void fogDistanceParam(FogDistanceParam& fp) {
        ImGui::Begin("Fog Parameter");

        ImGui::DragFloat("fogNear", &fp.m_Near, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("fogfar", &fp.m_Far, 0.001f);
        ImGui::DragFloat("fogDensity", &fp.m_Density, 0.001f);

        ImGui::End();
    }

    void color01(float temp[], int n) {
        for (int i = 0; i < n; i++) {
            temp[i] /= 255.0f;
        }
    }

    glm::vec3 vecColor3(float temp[]) {
        return glm::vec3(temp[0], temp[1], temp[2]);
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
