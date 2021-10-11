#pragma once
#include "Eigen/Dense"
#include <array>

class Camera {
public:
	Camera(unsigned int width, unsigned int height, float focalLength = 1) {
		memset(data, 0, sizeof(data));

		setCameraPosition({ 1, 0, 6 });
		setCameraDirection({ 0, 0, -1 });
		
		setUpDirection({ 0, 1 , 0 });
		setRightDirection({ 1, 0, 0 });

		float worldWidth = width * (1 / sqrt(width * height));
		float worldHeight = height * (1 / sqrt(width * height));
		float worldStep = worldWidth / width;
		setWorldStep(worldStep);
		setUpperLeft(Vector3f{ -worldWidth / 2, worldHeight / 2, -focalLength } + getCameraPosition());

		//for (int index = 0; index < dataSize; ++index) {
		//	data[index] = 1;
		//}
	
	}

	const Eigen::Vector3f getCameraPosition() {
		return Eigen::Map<const Eigen::Vector3f>(data);
	}

	void setCameraPosition(Eigen::Vector3f&& position) {
		Eigen::Map<Eigen::Vector3f>(data).array() = position;
	}

	const Eigen::Vector3f getCameraDirection() {
		return Eigen::Map<const Eigen::Vector3f>(data+4);
	}

	void setCameraDirection(Eigen::Vector3f&& direction) {
		Eigen::Map<Eigen::Vector3f>(data+4).array() = direction;
	}

	const Eigen::Vector3f getUpperLeft() {
		return Eigen::Map<const Eigen::Vector3f>(data+8);
	}

	void setUpperLeft(Eigen::Vector3f&& vector) {
		Eigen::Map<Eigen::Vector3f>(data+8).array() = vector;
	}


	const Eigen::Vector3f getUpDirection() {
		return Eigen::Map<const Eigen::Vector3f>(data+12);
	}

	void setUpDirection(Eigen::Vector3f&& vector) {
		Eigen::Map<Eigen::Vector3f>(data+12).array() = vector;
	}

	const Eigen::Vector3f getRightDirection() {
		return Eigen::Map<const Eigen::Vector3f>(data+16);
	}

	void setRightDirection(Eigen::Vector3f&& vector) {
		Eigen::Map<Eigen::Vector3f>(data+16).array() = vector;
	}

	float getWorldStep() {
		return data[20];
	}

	void setWorldStep(float worldStep) {
		data[19] = worldStep;
	}

	void printData() {
		for (int index = 0; index < dataSize; ++index) {
			std::cout << data[index] << " ";
		}
	}

	constexpr static unsigned int dataSize = 21;

	float data[dataSize];

private:
};