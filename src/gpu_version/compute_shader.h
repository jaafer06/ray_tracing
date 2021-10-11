#pragma once
#include <string>
#include "gpu_version/loadShader.h"
#include "glad/glad.h"


class ComputeShader {
public:
	const unsigned int programID;
	ComputeShader(std::string&& path, float* uniformBufferData=NULL) : programID (LoadShaders(GL_COMPUTE_SHADER, path.c_str())) {
		glUseProgram(programID);
		glGenBuffers(1, &uniformsBuffer);
		glBindBuffer(GL_UNIFORM_BUFFER, uniformsBuffer);
		glBufferData(GL_UNIFORM_BUFFER, 21*sizeof(float), uniformBufferData, GL_STATIC_DRAW); // allocate 152 bytes of memory
		glBindBuffer(GL_UNIFORM_BUFFER, 0);


		unsigned int coordinateIndex = glGetUniformBlockIndex(programID, "coordinates");
		glUniformBlockBinding(programID, coordinateIndex, 1);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, uniformsBuffer);
		//glBindBufferRange(GL_UNIFORM_BUFFER, 1, uniformsBuffer, 0, 21 * sizeof(float));
		glBindBuffer(GL_UNIFORM_BUFFER,0);
	}

	void compute(unsigned int x, unsigned int y, unsigned int z) {
		glDispatchCompute(x, y, z);
	}

	void setUniform(std::string&& name) {
		unsigned int location = glGetUniformLocation(programID, name.c_str());
		glUniform4fv(location, 1, nullptr);
	}

private:

	unsigned int uniformsBuffer;
};