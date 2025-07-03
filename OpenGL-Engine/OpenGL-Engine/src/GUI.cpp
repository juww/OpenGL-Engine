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
    void menuPlayAnimation(Animator& animator) {
        ImGui::Begin("play Animation");

        if (ImGui::Button("-0.001s")) {
            animator.animationTime -= 0.001f;
            if (animator.animationTime < 0.0f) {
                animator.animationTime = 0.0f;
            }
        }
        ImGui::SameLine();
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

        ImGui::SameLine();
        float progress = animator.animationTime, lengthTime = animator.animations[animator.currentAnimation].length;
        if (ImGui::Button("+0.001s")) {
            animator.animationTime += 0.001f;
            if (animator.animationTime > lengthTime) {
                animator.animationTime = lengthTime;
            }
        }
        char buf[32];
        sprintf(buf, "%.4f/%.4f", progress, lengthTime);
        ImGui::ProgressBar(progress / lengthTime, ImVec2(0.f, 0.f), buf);

        ImGui::End();
    }

    void modelAnimation(std::string name, Animator &animator) {

        ImGui::Begin(name.c_str());

        float progress = animator.animationTime, lengthTime = animator.animations[animator.currentAnimation].length;

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

        auto& nodes = animator.nodeAnimation[animator.currentAnimation];

        static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | 
            ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable 
            | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

        ImGui::CheckboxFlags("ImGuiTableFlags_ScrollY", &flags, ImGuiTableFlags_ScrollY);
        const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
        ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 6);

        for (int i = 0; i < nodes.size(); i++) {
            if (nodes[i].translate.empty() && nodes[i].rotate.empty() && nodes[i].scale.empty()) {
                continue;
            }
            ImGui::Text("node %d : %s\n", i, nodes[i].name.c_str());
            ImGui::Text("currTranslate : %f %f %f\n", animator.currentPose[i].pos.x, animator.currentPose[i].pos.y, animator.currentPose[i].pos.z);
            ImGui::Text("currRotate : %f %f %f %f\n", animator.currentPose[i].rotate.x, animator.currentPose[i].rotate.y, animator.currentPose[i].rotate.z, animator.currentPose[i].rotate.w);
            ImGui::Text("currScale : %f %f %f\n", animator.currentPose[i].scale.x, animator.currentPose[i].scale.y, animator.currentPose[i].scale.z);

            if (ImGui::BeginTable("table_scrolly", 3, flags, outer_size)) {
                ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
                /*ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_None);*/
                ImGui::TableSetupColumn("translation", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("rotation", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("scale", ImGuiTableColumnFlags_None);
                ImGui::TableHeadersRow();
                
                ImGui::TableNextRow();
                int mcol = std::max(nodes[i].translate.size(), std::max(nodes[i].rotate.size(), nodes[i].scale.size()));
                for (int j = 0; j < mcol; j++) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (j < nodes[i].translate.size()) {
                        bool vis = false;
                        if (progress >= nodes[i].translate[j].first) {
                            if (j + 1 < nodes[i].translate.size() && progress <= nodes[i].translate[j + 1].first) {
                                ImGui::TextColored(ImVec4(1, 1, 0, 1), "%f %f %f", nodes[i].translate[j].second.x, nodes[i].translate[j].second.y, nodes[i].translate[j].second.z);
                                vis = true;
                            }
                        }
                        if(vis == false) {
                            ImGui::Text("%f %f %f", nodes[i].translate[j].second.x, nodes[i].translate[j].second.y, nodes[i].translate[j].second.z);
                        }
                    } else {
                        ImGui::Text("-");
                    }
                    ImGui::TableNextColumn();
                    if (j < nodes[i].rotate.size()) {
                        bool vis = false;
                        if (progress >= nodes[i].rotate[j].first) {
                            if (j + 1 < nodes[i].rotate.size() && progress <= nodes[i].rotate[j + 1].first) {
                                ImGui::TextColored(ImVec4(1, 1, 0, 1), "%f %f %f %f", nodes[i].rotate[j].second.x, nodes[i].rotate[j].second.y, nodes[i].rotate[j].second.z, nodes[i].rotate[j].second.w);
                                vis = true;
                            }
                        }
                        if (vis == false) {
                            ImGui::Text("%f %f %f %f", nodes[i].rotate[j].second.x, nodes[i].rotate[j].second.y, nodes[i].rotate[j].second.z, nodes[i].rotate[j].second.w);
                        }
                    } else {
                        ImGui::Text("-");
                    }
                    ImGui::TableNextColumn();
                    if (j < nodes[i].scale.size()) {
                        bool vis = false;
                        if (progress >= nodes[i].scale[j].first) {
                            if (j + 1 < nodes[i].scale.size() && progress <= nodes[i].scale[j + 1].first) {
                                ImGui::TextColored(ImVec4(1, 1, 0, 1), "%f %f %f", nodes[i].scale[j].second.x, nodes[i].scale[j].second.y, nodes[i].scale[j].second.z);
                                vis = true;
                            }
                        }
                        if (vis == false) {
                            ImGui::Text("%f %f %f", nodes[i].scale[j].second.x, nodes[i].scale[j].second.y, nodes[i].scale[j].second.z);
                        }
                    }
                    else {
                        ImGui::Text("-");
                    }
                }
                ImGui::EndTable();
            }
        }

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
