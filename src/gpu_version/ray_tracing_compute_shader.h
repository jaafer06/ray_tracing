#pragma once
#include <string>
#include "gpu_version/loadShader.h"
#include "glad/glad.h"
#include "gpu_version/camera.h"
#include "gpu_version/shape.h"
#include <vector>

class RayTracingComputeShader {
public:
	const unsigned int programID;
	RayTracingComputeShader(Camera& camera, unsigned int programID) 
			: programID (programID), camera(camera) {
		glUseProgram(programID);
		glGenBuffers(1, &cameraPositionBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, cameraPositionBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * camera.dataSize, camera.data, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cameraPositionBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

		glGenBuffers(1, &shapesBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, shapesBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Shape) * shapes.size(), shapes.data(), GL_DYNAMIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shapesBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	}

	void updateCameraBuffer() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, cameraPositionBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * camera.dataSize, camera.data, GL_DYNAMIC_DRAW);
	}

	void compute(unsigned int x, unsigned int y, unsigned int z) {
		glUseProgram(programID);
		glDispatchCompute(x, y, z);
	}

	void updateShapeBuffer() {
		glUseProgram(programID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, shapesBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Shape) * shapes.size(), shapes.data(), GL_DYNAMIC_DRAW);
		int location = glGetUniformLocation(programID, "shapeCount");
		glUniform1ui(location, unsigned int(shapes.size()));
	}

	std::vector<Shape> shapes;

private:
	Camera& camera;
	unsigned int shapesBuffer;
	unsigned int cameraPositionBuffer;
};