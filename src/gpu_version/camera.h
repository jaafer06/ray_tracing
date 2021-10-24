#pragma once
#include "Eigen/Dense"
#include <array>
#include "glad/glad.h"
#include <functional>
#include "GLFW/glfw3.h"

class Camera {
public:
	enum class Direction {
		FORWARD, BACKWARD, RIGHT, LEFT
	};

	Camera(unsigned int width, unsigned int height, float focalLength = 1) : focalLength(focalLength) {
		memset(data, 0, sizeof(data));

		setCameraPosition({ 1, 1, 6 });
		setCameraDirection({ 0, 0, -1 });

		setUpDirection({ 0, 1 , 0 });
		setRightDirection({ 1, 0, 0 });

		worldWidth = width * (1 / sqrt(width * height));
		worldHeight = height * (1 / sqrt(width * height));
		setWorldStep(worldWidth / width);
		setUpperLeft(Vector3f{ -worldWidth / 2, worldHeight / 2, -focalLength } + getCameraPosition());

	}

	const Eigen::Vector3f getCameraPosition() {
		return Eigen::Map<const Eigen::Vector3f>(data);
	}

	void setCameraPosition(Eigen::Vector3f&& position) {
		Eigen::Map<Eigen::Vector3f>(data).array() = position;
	}

	const Eigen::Vector3f getCameraDirection() {
		return Eigen::Map<const Eigen::Vector3f>(data + 4);
	}

	void setCameraDirection(Eigen::Vector3f&& direction) {
		Eigen::Map<Eigen::Vector3f>(data + 4).array() = direction;
	}

	const Eigen::Vector3f getUpperLeft() {
		return Eigen::Map<const Eigen::Vector3f>(data + 8);
	}

	void setUpperLeft(Eigen::Vector3f&& vector) {
		Eigen::Map<Eigen::Vector3f>(data + 8).array() = vector;
	}


	const Eigen::Vector3f getUpDirection() {
		return Eigen::Map<const Eigen::Vector3f>(data + 12);
	}

	void setUpDirection(Eigen::Vector3f&& vector) {
		Eigen::Map<Eigen::Vector3f>(data + 12).array() = vector;
	}

	const Eigen::Vector3f getRightDirection() {
		return Eigen::Map<const Eigen::Vector3f>(data + 16);
	}

	void setRightDirection(Eigen::Vector3f&& vector) {
		Eigen::Map<Eigen::Vector3f>(data + 16).array() = vector;
	}

	float getWorldStep() {
		return data[19];
	}

	void setWorldStep(float worldStep) {
		data[19] = worldStep;
	}

	void printData() {
		for (int index = 0; index < dataSize; ++index) {
			std::cout << data[index] << " ";
		}
	}

	void rotate(float horizontal, float vertical) {
		auto rotation = Eigen::AngleAxisf(horizontal, getUpDirection()).toRotationMatrix();
		rotation = (Eigen::AngleAxisf(vertical, rotation*getRightDirection())).toRotationMatrix() * rotation;
		setUpDirection(rotation * getUpDirection());
		setRightDirection(rotation * getRightDirection());
		setCameraDirection(rotation * getCameraDirection());
		setUpperLeft((-worldWidth / 2 * getRightDirection()) + (worldHeight / 2 * getUpDirection()) + getCameraDirection() * focalLength + getCameraPosition());
	}

	void move(Direction direction) {
		switch (direction)
		{
		case Direction::FORWARD:
			setCameraPosition(getCameraPosition() + getCameraDirection() * 0.2);
			break;
		case Direction::BACKWARD:
			setCameraPosition(getCameraPosition() - getCameraDirection() * 0.2);
			break;
		case Direction::RIGHT:
			setCameraPosition(getCameraPosition() + getRightDirection() * 0.2);
			break;
		case Direction::LEFT:
			setCameraPosition(getCameraPosition() - getRightDirection() * 0.2);
			break;
		default:
			break;
		}
		setUpperLeft((-worldWidth / 2 * getRightDirection()) + (worldHeight / 2 * getUpDirection()) + getCameraDirection() * focalLength + getCameraPosition());
	}

	constexpr static unsigned int dataSize = 21;

	float data[dataSize];
	float worldWidth, worldHeight;
	float focalLength;
	bool cameraMode = false;
private:
};