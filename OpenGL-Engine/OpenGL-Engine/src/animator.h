#pragma once
#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include<string>
#include<cstring>
#include<vector>
#include<map>
#include<unordered_map>
#include<cmath>

#include "interpolate.h"
#include "transformation.h"

class NodeAnimation {
public:
    std::string name;
    std::vector<std::pair<float, glm::vec3> > translate;
    std::vector<std::pair<float, glm::quat> > quat;
    std::vector<std::pair<float, glm::vec3> > scale;

    NodeAnimation() {
        name = "";
        translate.clear();
        quat.clear();
        scale.clear();
    }

    NodeAnimation& operator=(const NodeAnimation &node) {
        name = node.name;
        translate = node.translate;
        quat = node.quat;
        scale = node.scale;

        return *this;
    }
};


class KeyFrame {

public:
	float Timestamp;
	// node/joint - jointTransform
	std::unordered_map<int, Transformation> poseTransform;
	std::string Interpolation;

	KeyFrame(const float &_t, const int &targetNode) {
		Timestamp = _t;
		if (poseTransform.find(targetNode) == poseTransform.end()) {
			Transformation transform;
			poseTransform.insert({ targetNode,transform });
		}
	}

	~KeyFrame() {
        poseTransform.clear();
	}

	void JointTransform(const glm::vec4 &transform, const int targetNode, const std::string targetPath) {
		auto itr = poseTransform.find(targetNode);
		if (itr == poseTransform.end()) {
			Transformation transform;
			poseTransform.insert({ targetNode,transform });
			itr = poseTransform.find(targetNode);
		}
		Transformation& pose = itr->second;

		if (targetPath == "rotation") {
            pose.quaternion.w = transform.w;
            pose.quaternion.x = transform.x;
            pose.quaternion.y = transform.y;
            pose.quaternion.z = transform.z;
		}
		if (targetPath == "translation") {
			for (int i = 0; i < 3; i++) {
				pose.pos[i] = transform[i];
			}
		}
		if (targetPath == "scale") {
			for (int i = 0; i < 3; i++) {
				pose.scale[i] = transform[i];
			}
		}
        // i dunno?
        // 
		//if (targetPath == "weights") {
		//	pose.weight = transform[0];
		//}
	}

};

class Animation {

public:
    std::string name;
    
	float startTime, length;
	int count = 0;
	std::map<unsigned int, unsigned int> timestamp; //timeframe -> index time;
	std::vector<KeyFrame> keyframes;

	Animation() {
		length = 0.0f;
        startTime = 1e9f;
		count = 0;
		timestamp.clear();
        keyframes.clear();
	}

	~Animation() {
        length = 0.0f;
        startTime = 0.0f;
		count = -1;
		timestamp.clear();
        keyframes.clear();
	}

	void addKeyframe(const std::vector<float>& inputData, const std::vector<glm::vec4>& outputData,
		const int targetNode, const std::string& interpolation, const std::string& targetPath) {

		int n = inputData.size();
		for (int i = 0; i < n; i++) {
			unsigned int t = (inputData[i] * 1e4);

			std::map<unsigned int, unsigned int>::iterator itr = timestamp.find(t);			
			if (itr == timestamp.end()) {
				keyframes.push_back(KeyFrame(inputData[i], targetNode));
                timestamp.insert({ t, keyframes.size() - 1 });
				count++;
			    itr = timestamp.find(t);
			}

			KeyFrame& keyframe = keyframes[itr->second];
			if (length < inputData[i]) {
				length = inputData[i];
			}

            if (startTime > inputData[i]) {
                startTime = inputData[i];
            }
			keyframe.JointTransform(outputData[i], targetNode, targetPath);
		}
	}

    void fillMissingKeyframes(const std::vector<int>& skin, const std::vector<NodeAnimation>& nodeAnimations,
        const std::vector<Transformation>& nodeDefault
        ) {

        for (int i = 0; i < skin.size(); i++) {
            int bone = skin[i];
            for (auto& temp : timestamp) {
                KeyFrame& keyframe = keyframes[temp.second];
                bool filling = false;
                auto poseTransform = keyframe.poseTransform.find(bone);
                if (poseTransform == keyframe.poseTransform.end()) {
                    filling = true;
                    keyframe.poseTransform.insert({ bone, nodeDefault[bone] });
                    poseTransform = keyframe.poseTransform.find(bone);
                }
                const NodeAnimation& node = nodeAnimations[bone];

                glm::vec3 resTranslate = nodeDefault[bone].pos;
                glm::quat resQuat = nodeDefault[bone].quaternion;
                glm::vec3 resScale = nodeDefault[bone].scale;
                //fill translate
                bool flag = false;
                for (int j = 0; j < node.translate.size(); j++) {
                    if (flag == true) break;

                    if (abs(node.translate[j].first - keyframe.Timestamp) <= 1e-4f) {
                        flag = true;
                        resTranslate = glm::vec3(node.translate[j].second.x, node.translate[j].second.y, node.translate[j].second.z);
                        break;
                    }
                    if (node.translate[j].first > keyframe.Timestamp && flag == false) {
                        if (j - 1 >= 0) {
                            float ta = node.translate[j - 1].first;
                            float tb = node.translate[j].first;
                            float tk = keyframe.Timestamp;
                            float tt = (tk - ta) / (tb - ta);

                            glm::vec3 TA = node.translate[j - 1].second;
                            glm::vec3 TB = node.translate[j].second;
                            glm::vec3 res = interpolate::lerp(TA, TB, tt);
                            resTranslate = res;
                            flag = true;
                            break;
                        }
                    }
                }
                keyframe.poseTransform[bone].pos = resTranslate;

                //fill rotate
                flag = false;
                for (int j = 0; j < node.quat.size(); j++) {
                    if (flag == true) break;

                    if (abs(node.quat[j].first - keyframe.Timestamp) <= 1e-4f) {
                        flag = true;
                        resQuat = node.quat[j].second;
                        break;
                    }
                    if (node.quat[j].first > keyframe.Timestamp && flag == false) {
                        if (j - 1 >= 0) {
                            float ta = node.quat[j - 1].first;
                            float tb = node.quat[j].first;
                            float tk = keyframe.Timestamp;
                            float tt = (tk - ta) / (tb - ta);
                             
                            glm::quat QA = node.quat[j - 1].second;
                            glm::quat QB = node.quat[j].second;
                            resQuat = glm::slerp(QA, QB, tt);

                            flag = true;
                            break;
                        }
                    }
                }
                keyframe.poseTransform[bone].quaternion = resQuat;
                //fill scale
                flag = false;
                for (int j = 0; j < node.scale.size(); j++) {
                    if (flag == true) break;

                    if (abs(node.scale[j].first - keyframe.Timestamp) <= 1e-4f) {
                        flag = true;
                        resScale = glm::vec3(node.scale[j].second.x, node.scale[j].second.y, node.scale[j].second.z);
                        break;
                    }
                    if (node.scale[j].first > keyframe.Timestamp && flag == false) {
                        if (j - 1 >= 0) {
                            float ta = node.scale[j - 1].first;
                            float tb = node.scale[j].first;
                            float tk = keyframe.Timestamp;
                            float tt = (tk - ta) / (tb - ta);

                            glm::vec3 TA = node.scale[j - 1].second;
                            glm::vec3 TB = node.scale[j].second;
                            glm::vec3 res = interpolate::lerp(TA, TB, tt);

                            resScale = res;
                            flag = true;
                            break;
                        }
                    }
                }
                keyframe.poseTransform[bone].scale = resScale;
            }
        }
    }
};

class Animator {

public:
	int currentAnimation;
    bool play;
	float animationTime, lastTime;
	std::vector<Animation> animations;

	std::map<int, Transformation> currentPose;
    int currentKeyframe, nextKeyframe;
    std::vector<int> IndexKeyframes;

    std::vector<std::vector<NodeAnimation> > nodeAnimation;
    std::vector<Transformation> nodeDefaultTransform;

	Animator() {
		currentAnimation = -1;
		currentKeyframe = 0;
		nextKeyframe = 0;
		animationTime = 0.0f;
		lastTime = 0.0f;
		animations.clear();
        play = false;
        IndexKeyframes.clear();
        nodeAnimation.clear();
	}

    void reserveSizeNodeAnimation(int n, int m) {
        nodeAnimation.resize(n);
        animations.resize(n);
        for (int i = 0; i < n; i++) {
            nodeAnimation[i].resize(m);
        }
        nodeDefaultTransform.resize(m);
    }

    void fillNodeAnimation(int animation, int node,
        std::vector<float> input, std::vector<glm::vec4> output,
        std::string targetPath, std::string nodeName) {

        NodeAnimation& t_na = nodeAnimation[animation][node];
        t_na.name = nodeName;
        int nn = input.size();
        for (int i = 0; i < nn; i++) {
            if (targetPath == "rotation") {
                glm::quat qtemp(output[i].w, output[i].x, output[i].y, output[i].z);
                t_na.quat.push_back({ input[i], qtemp });
            }
            if (targetPath == "translation") {
                t_na.translate.push_back({ input[i], output[i] });
            }
            if (targetPath == "scale") {
                t_na.scale.push_back({ input[i], output[i] });
            }

        }
    }

	~Animator() {
	}

	void doAnimation(int animation) {

        if (animation >= animations.size() || animations.empty()) return;

		currentAnimation = animation;
		animationTime = animations[currentAnimation].startTime;
		lastTime = static_cast<float>(glfwGetTime());
		currentKeyframe = 0;
		nextKeyframe = currentKeyframe + 1;
        IndexKeyframes.clear();
        play = true;

        for (auto& timestamp : animations[currentAnimation].timestamp) {
            IndexKeyframes.push_back(timestamp.second);
        }
	}

	bool update(float deltaTime) {
		if (currentAnimation < 0 || currentAnimation >= animations.size()) return false;
        if (play == false) {
            deltaTime = 0.0f;
        }

		increaseAnimationTime(deltaTime);
		calculateCurrentAnimationPose();
		return true;
	}

private:
	void increaseAnimationTime(float deltaTime) {

		Animation& animation = animations[currentAnimation];
		animationTime += deltaTime;

		if (animationTime > animation.length || animationTime < animations[currentAnimation].startTime) {
			animationTime = animations[currentAnimation].startTime;
			currentKeyframe = 0;
			nextKeyframe = currentKeyframe + 1;
		}
	}

	void calculateCurrentAnimationPose() {
		Animation& animation = animations[currentAnimation];
		std::vector<KeyFrame>& kf = animation.keyframes;

		while (animationTime > kf[IndexKeyframes[nextKeyframe]].Timestamp) {
            if (nextKeyframe >= IndexKeyframes.size()) {
                nextKeyframe = IndexKeyframes.size() - 1;
                break;
            }
			currentKeyframe = nextKeyframe;
			nextKeyframe++;
		}
        int curr = IndexKeyframes[currentKeyframe];
        int next = IndexKeyframes[nextKeyframe];
		float progression = calculateProgression(kf[curr],kf[next]);
		interpolatePose(kf[curr], kf[next], progression);
	}

	float calculateProgression(KeyFrame& currentFrame, KeyFrame& nextFrame) {
		float totalTime = nextFrame.Timestamp - currentFrame.Timestamp;
		float currentTime = animationTime - currentFrame.Timestamp;
		float progression = currentTime / totalTime;
		return progression;
	}

	void interpolatePose(KeyFrame& currentFrame, KeyFrame& nextFrame, float progression) {

		for (auto &t : currentFrame.poseTransform) {
			Transformation& previousTransform = t.second;
			std::unordered_map<int, Transformation>::iterator itr = nextFrame.poseTransform.find(t.first);
			if (itr == nextFrame.poseTransform.end()) {
				nextFrame.poseTransform[t.first] = t.second;
				itr = nextFrame.poseTransform.find(t.first);
			}
			Transformation& nextTransform = itr->second;
			Transformation currentTransform = interpolate(previousTransform, nextTransform, progression);
			currentPose[t.first] = currentTransform;
		}
	}

	Transformation interpolate(Transformation& previousTransform, Transformation& nextTransform, float& progression) {
		Transformation result;

		result.pos = interpolate::lerp(previousTransform.pos, nextTransform.pos, progression);
        result.quaternion = glm::slerp(previousTransform.quaternion, nextTransform.quaternion, progression);
		result.scale = interpolate::lerp(previousTransform.scale, nextTransform.scale, progression);

		return result;
	}
};

#endif
