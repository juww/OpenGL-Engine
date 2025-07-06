#pragma once
#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
    std::vector<std::pair<float, glm::vec4> > translate;
    std::vector<std::pair<float, glm::vec4> > rotate;
    std::vector<std::pair<float, glm::vec4> > scale;

    NodeAnimation() {
        name = "";
        translate.clear();
        rotate.clear();
        scale.clear();
    }

    NodeAnimation& operator=(const NodeAnimation &node) {
        name = node.name;
        translate = node.translate;
        rotate = node.rotate;
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
	}

	void JointTransform(const glm::vec4 &transform, const int targetNode, const std::string targetPath) {
		auto itr = poseTransform.find(targetNode);
		if (itr == poseTransform.end()) {
			Transformation transform;
			poseTransform.insert({ targetNode,transform });
			itr = poseTransform.find(targetNode);
		}
		//std::cout << "targetnode : " << targetNode << "\n";
		//std::cout << "targetpath : " << targetPath << "\n";
		//for (int i = 0; i < 4; i++) {
		//	printf("%f ", transform[i]);
		//}
		//printf("\n");
		Transformation& pose = itr->second;
		//std::cout << "\n";

		if (targetPath == "rotation") {
			for (int i = 0; i < 4; i++) {
                pose.rotate[(i + 1) % 4] = transform[i];
			}
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
        startTime = 1000.0f;
		count = 0;
		timestamp.clear();
	};

	~Animation() {
        length = 0.0f;
        startTime = 1000.0f;
		count = -1;
		timestamp.clear();
	};

	void addKeyframe(const std::vector<float>& inputData, const std::vector<glm::vec4>& outputData,
		const int targetNode, const std::string& interpolation, const std::string& targetPath) {

		int n = inputData.size();
		for (int i = 0; i < n; i++) {
			unsigned int t = (inputData[i] * 1e4);
			std::map<unsigned int, unsigned int>::iterator itr = timestamp.find(t);
			//std::cout << "tt >> " << t << std::endl;
			if (itr == timestamp.end()) {
				keyframes.push_back(KeyFrame(inputData[i], targetNode));
                timestamp.insert({ t, keyframes.size() - 1 });
				//std::cout << "is not found and create count "<<count << "\n";
				count++;
			    itr = timestamp.find(t);
			}
			//std::cout << "itr->first " << itr->first << std::endl;
			//std::cout << "itr->second " << itr->second << std::endl;
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
                    // WIP: todo
                    // fill the missing keyframe with interpolate between them.
                    // if the node is empty keyframe, perhaps make flags isempty.
                    // 
                }
                const NodeAnimation& node = nodeAnimations[bone];

                glm::vec3 resTranslate = nodeDefault[bone].pos;
                glm::vec4 resRotate = nodeDefault[bone].rotate;
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
                for (int j = 0; j < node.rotate.size(); j++) {
                    if (flag == true) break;

                    if (abs(node.rotate[j].first - keyframe.Timestamp) <= 1e-4f) {
                        flag = true;
                        resRotate = node.rotate[j].second;
                        break;
                    }
                    if (node.rotate[j].first > keyframe.Timestamp && flag == false) {
                        if (j - 1 >= 0) {
                            float ta = node.rotate[j - 1].first;
                            float tb = node.rotate[j].first;
                            float tk = keyframe.Timestamp;
                            float tt = (tk - ta) / (tb - ta);

                            glm::vec4 TA = node.rotate[j - 1].second;
                            glm::vec4 TB = node.rotate[j].second;
                            glm::vec4 res = interpolate::slerp(TA, TB, tt);
                            resRotate = res;

                            flag = true;
                            break;
                        }
                    }
                }
                keyframe.poseTransform[bone].rotate = resRotate;
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
    bool playAnimation;
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
        playAnimation = false;
        IndexKeyframes.clear();
        nodeAnimation.clear();
	}

    void reserveSizeNodeAnimation(int n, int m) {
        nodeAnimation.resize(n);
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
                glm::vec4 temp = output[i];
                for (int j = 0; j < 4; j++) {
                    output[i][(j + 1) % 4] = temp[j];
                }
                t_na.rotate.push_back({ input[i], output[i] });
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
		currentAnimation = animation;
		animationTime = animations[currentAnimation].startTime;
		lastTime = static_cast<float>(glfwGetTime());
		currentKeyframe = 0;
		nextKeyframe = currentKeyframe + 1;
		//currentPose.clear();
        IndexKeyframes.clear();
        playAnimation = true;

        for (auto& timestamp : animations[currentAnimation].timestamp) {
            IndexKeyframes.push_back(timestamp.second);
        }
	}

	bool update(float deltaTime) {
		if (currentAnimation < 0 || currentAnimation >= animations.size()) return false;
        if (playAnimation == false) {
            deltaTime = 0.0f;
        }

		increaseAnimationTime(deltaTime);
		calculateCurrentAnimationPose();
		return true;
	}

private:
	void increaseAnimationTime(float deltaTime) {
		Animation& animation = animations[currentAnimation];
		//std::cout << "before : " << animationTime << std::endl;
		animationTime += deltaTime;
		//std::cout << "animation time: " << animationTime << std::endl;

		if (animationTime > animation.length || animationTime < animations[currentAnimation].startTime) {
			//std::cout << "reset: " << std::endl;
			//printf("cur = %f --- length = %f\n", animationTime, animation.length);
			animationTime = animations[currentAnimation].startTime;
			currentKeyframe = 0;
			nextKeyframe = currentKeyframe + 1;
		}
	}

	void calculateCurrentAnimationPose() {
		Animation& animation = animations[currentAnimation];
		std::vector<KeyFrame>& kf = animation.keyframes;
		//printf("cur = %f --- kf = %f\n", animationTime, kf[nextKeyframe].Timestamp);
        
		while (animationTime > kf[IndexKeyframes[nextKeyframe]].Timestamp) {
            if (nextKeyframe >= IndexKeyframes.size()) {
                nextKeyframe = IndexKeyframes.size() - 1;
                break;
            }
			currentKeyframe = nextKeyframe;
			//std::cout << "current Frame: " << currentKeyframe << std::endl;
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
				//nanti di benerin
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
		result.rotate = interpolate::slerp(previousTransform.rotate, nextTransform.rotate, progression);

		//nanti
		result.scale = interpolate::lerp(previousTransform.scale, nextTransform.scale, progression);

		return result;
	}
};

#endif
