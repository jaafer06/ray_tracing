#pragma once
#include <string>
#include "gpu_version/loadShader.h"
#include "glad/glad.h"
#include "gpu_version/camera.h"

class RayTracingComputeShader {
public:
	const unsigned int programID;
	RayTracingComputeShader(Camera& camera, unsigned int programID) 
			: programID (programID), camera(camera) {

		glUseProgram(programID);
		int location = glGetUniformLocation(programID, "cameraPosition");
		glUniform3fv(location, 1, camera.getCameraPosition().data());
		location = glGetUniformLocation(programID, "cameraDirection");
		glUniform3fv(location, 1, camera.getCameraDirection().data());

		location = glGetUniformLocation(programID, "upperLeft");
		glUniform3fv(location, 1, camera.getUpperLeft().data());

		location = glGetUniformLocation(programID, "up");
		glUniform3fv(location, 1, camera.getUpDirection().data());

		location = glGetUniformLocation(programID, "right");
		glUniform3fv(location, 1, camera.getRightDirection().data());

		location = glGetUniformLocation(programID, "worldStep");
		glUniform1f(location, camera.getWorldStep());

	}

	void updateCameraCoordinates() {
		glUseProgram(programID);
		int location = glGetUniformLocation(programID, "cameraPosition");
		glUniform3fv(location, 1, camera.getCameraPosition().data());

		location = glGetUniformLocation(programID, "cameraDirection");
		glUniform3fv(location, 1, camera.getCameraDirection().data());

		location = glGetUniformLocation(programID, "upperLeft");
		glUniform3fv(location, 1, camera.getUpperLeft().data());

		location = glGetUniformLocation(programID, "up");
		glUniform3fv(location, 1, camera.getUpDirection().data());

		location = glGetUniformLocation(programID, "right");
		glUniform3fv(location, 1, camera.getRightDirection().data());

		location = glGetUniformLocation(programID, "worldStep");
		glUniform1f(location, camera.getWorldStep());

	}

	void compute(unsigned int x, unsigned int y, unsigned int z) {
		glUseProgram(programID);
		glDispatchCompute(x, y, z);
	}

	void setUniform(std::string&& name) {

	}

private:
	unsigned int uniformsBuffer;
	Camera& camera;
};