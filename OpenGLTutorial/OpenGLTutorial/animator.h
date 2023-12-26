#pragma once
#ifndef ANIMATOR_H
#define ANIMATOR_H

#include<string>
#include<cstring>
#include<vector>
#include<map>
#include<unordered_map>

struct Transformation {
	glm::mat4 mat;

	glm::vec4 rotation;
	glm::vec3 translation;
	glm::vec3 scalation;
	float weight;

	Transformation() {
		mat = glm::mat4(1.0f);

		rotation = glm::vec4(0.0f);
		translation = glm::vec3(0.0f);
		scalation = glm::vec3(1.0f);
		weight = 0.0f;
	}

	~Transformation() {

	}
};

class KeyFrame {

public:
	float Timestamp;
	// node/joint - jointTransform
	std::unordered_map<int, Transformation> poseTransform;
	std::string Interpolation;

	KeyFrame(const float &_t, const int &nodeTarget) {
		Timestamp = _t;
		if (poseTransform.find(nodeTarget) == poseTransform.end()) {
			Transformation transform;
			poseTransform.insert({nodeTarget,transform});
		}
	}

	~KeyFrame() {
	}

	void JointTransform(const glm::vec4 &transform, const int targetNode, const std::string targetPath) {
		auto itr = poseTransform.find(targetNode);
		std::cout << "targetnode : " << targetNode << "\n";
		std::cout << "targetpath : " << targetPath << "\n";
		Transformation& pose = itr->second;
		if (targetPath == "rotation") {
			for (int i = 0; i < 4; i++) {
				pose.rotation[(i + 1) % 4] = transform[i];
			}
		}
		if (targetPath == "translation") {
			for (int i = 0; i < 3; i++) {
				pose.translation[i] = transform[i];
			}
		}
		if (targetPath == "scale") {
			for (int i = 0; i < 3; i++) {
				pose.scalation[i] = transform[i];
			}
		}
		if (targetPath == "weights") {
			pose.weight = transform[0];
		}
	}

};

class Animation {

public:

	float length;
	int count = 0;
	std::map<unsigned int, unsigned int> timestamp; //timeframe -> index time;
	std::vector<KeyFrame> keyframes;

	Animation() {
		length = 0.0;
		count = 0;
		timestamp.clear();
	};

	~Animation() {
		length = -1.0;
		count = -1;
		timestamp.clear();
	};


	void addKeyframe(const std::vector<float>& inputData, const std::vector<glm::vec4>& outputData,
		const int targetNode, const std::string& interpolation, const std::string& targetPath) {

		int n = inputData.size();
		for (int i = 0; i < n; i++) {
			unsigned int t = (inputData[i] * 1e7);
			std::map<unsigned int, unsigned int>::iterator itr = timestamp.find(t);
			if (itr == timestamp.end()) {
				timestamp.insert({ t,count });
				keyframes.push_back(KeyFrame(inputData[i], targetNode));
				count++;
			}

			itr = timestamp.find(t);
			KeyFrame& keyframe = keyframes[itr->second];
			keyframe.JointTransform(outputData[i], targetNode, targetPath);
		}

	}
};

class Animator {

public:
	int currentAnimation;
	std::vector<Animation> animations;
	float animationTime;

	Animator() {

	}

	~Animator() {

	}
};


#endif