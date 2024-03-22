#pragma once
#ifndef ANIMATOR_H
#define ANIMATOR_H

#include<string>
#include<cstring>
#include<vector>
#include<map>
#include<unordered_map>
#include<cmath>

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

	inline Transformation operator=(const Transformation& b) {
		mat = b.mat;
		rotation = b.rotation;
		translation = b.translation;
		scalation = b.scalation;
		weight = b.weight;

		return b;
	}

	~Transformation() {

	}

	glm::vec3 lerp(glm::vec3 v0, glm::vec3 v1, float t) {
		return ((1.0f - t) * v0) + (t * v1);
	}

	glm::vec3 lerp(float t, glm::vec3 v0, glm::vec3 v1) {
		return v0 + (t * (v1 - v0));
	}

	glm::vec4 lerp(float t, glm::vec4 v0, glm::vec4 v1) {
		return v0 + (t * (v1 - v0));
	}

	glm::vec4 slerp(glm::vec4 v0, glm::vec4 v1, float t) {

		// use normalize quaternion, calculation become wrong (result is nan);
		//v0 = glm::normalize(v0);
		//v1 = glm::normalize(v1);
		float dotq = glm::dot(v0, v1);
		if (dotq < 0) {
			v1 = -v1;
			dotq = glm::dot(v0, v1);
		}
		float angle = glm::acos(dotq);
		float sinAngle = glm::sin(angle);

		if (sinAngle == 0) {
			return lerp(t, v0, v1);
		}

		glm::vec4 q0 = (sin((1.0f - t) * angle) / sinAngle) * v0;
		glm::vec4 q1 = (sin(t * angle) / sinAngle) * v1;

		return q0 + q1;
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
			//std::cout << "tt >> " << t << std::endl;
			if (itr == timestamp.end()) {
				timestamp.insert({ t,count });
				//std::cout << "is not found and create count "<<count << "\n";
				keyframes.push_back(KeyFrame(inputData[i], targetNode));
				count++;
			}
			itr = timestamp.find(t);
			//std::cout << "itr->first " << itr->first << std::endl;
			//std::cout << "itr->second " << itr->second << std::endl;
			KeyFrame& keyframe = keyframes[itr->second];
			if (length < inputData[i]) {
				length = inputData[i];
			}
			keyframe.JointTransform(outputData[i], targetNode, targetPath);
		}
	}
};

class Animator {

public:
	int currentAnimation;
	std::map<int, Transformation> currentPose;
	int currentKeyframe, nextKeyframe;
	std::vector<Animation> animations;
	float animationTime, lastTime;


	Animator() {
		currentAnimation = -1;
		currentKeyframe = -1;
		nextKeyframe = -1;
		animationTime = 0.0f;
		lastTime = 0.0f;
		animations.clear();
	}

	~Animator() {
	}

	void doAnimation(int animation) {
		currentAnimation = animation;
		animationTime = 0.0f;
		lastTime = static_cast<float>(glfwGetTime());
		currentKeyframe = 0;
		nextKeyframe = currentKeyframe + 1;
		currentPose.clear();
	}

	bool update(float deltaTime) {
		if (currentAnimation < 0 || currentAnimation >= animations.size()) return false;

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
		if (animationTime > animation.length) {
			//std::cout << "reset: " << std::endl;
			//printf("cur = %f --- length = %f\n", animationTime, animation.length);
			animationTime = 0.0f;
			currentKeyframe = 0;
			nextKeyframe = currentKeyframe + 1;
		}
	}

	void calculateCurrentAnimationPose() {
		Animation& animation = animations[currentAnimation];
		std::vector<KeyFrame>& kf = animation.keyframes;
		//printf("cur = %f --- kf = %f\n", animationTime, kf[nextKeyframe].Timestamp);
		if (animationTime > kf[nextKeyframe].Timestamp) {
			currentKeyframe = nextKeyframe;
			//std::cout << "current Frame: " << currentKeyframe << std::endl;
			nextKeyframe++;
			if (nextKeyframe >= kf.size()) nextKeyframe = kf.size() - 1;
		}
		float progression = calculateProgression(kf[currentKeyframe],kf[nextKeyframe]);
		interpolatePose(kf[currentKeyframe], kf[nextKeyframe], progression);
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

		result.translation = result.lerp(previousTransform.translation, nextTransform.translation, progression);
		result.rotation = result.slerp(previousTransform.rotation, nextTransform.rotation, progression);

		//nanti
		//result.scalation = result.lerp(previousTransform.scalation, nextTransform.scalation, progression);

		return result;
	}
};


#endif
