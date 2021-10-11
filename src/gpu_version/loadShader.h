#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include "glad/glad.h"

using namespace std;

unsigned int CompileShader(unsigned int type, const char* shader_file_path) {
	// Create the shaders
	GLuint ShaderID = glCreateShader(type);

	// Read the Vertex Shader code from the file
	std::string ShaderCode;
	std::ifstream ShaderStream(shader_file_path, std::ios::in);
	if (ShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << ShaderStream.rdbuf();
		ShaderCode = sstr.str();
		ShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", shader_file_path);
		getchar();
		return 0;
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Shader
	printf("Compiling shader : %s\n", shader_file_path);
	char const* ShaderSourcePointer = ShaderCode.c_str();
	glShaderSource(ShaderID, 1, &ShaderSourcePointer, NULL);
	glCompileShader(ShaderID);

	// Check Vertex Shader
	glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ShaderIDErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(ShaderID, InfoLogLength, NULL, &ShaderIDErrorMessage[0]);
		printf("%s\n", &ShaderIDErrorMessage[0]);
	}

	return ShaderID;
}


struct ProgramIDs {
	const unsigned int ray_tracing;
	const unsigned int converter;
};

ProgramIDs LoadShaders() {


	unsigned int RayTracingShader = CompileShader(GL_COMPUTE_SHADER, "../src/shaders/RayTracing.glsl");

	// Link the program
	printf("Linking program\n");
	GLuint RayTracingProgramID = glCreateProgram();
	glAttachShader(RayTracingProgramID, RayTracingShader);
	glLinkProgram(RayTracingProgramID);

	// Check the program
	int Result;
	int InfoLogLength;
	glGetProgramiv(RayTracingProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(RayTracingProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(RayTracingProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	unsigned int ConvertShader = CompileShader(GL_COMPUTE_SHADER, "../src/shaders/PixelConversion.glsl");
	GLuint ConverterProgramID = glCreateProgram();
	glAttachShader(ConverterProgramID, ConvertShader);
	printf("Linking program\n");
	glLinkProgram(ConverterProgramID);
	glGetProgramiv(ConverterProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ConverterProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ConverterProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(RayTracingProgramID, RayTracingShader);
	glDetachShader(ConverterProgramID, ConvertShader);
	glDeleteShader(RayTracingShader);
	glDeleteShader(ConvertShader);

	return { RayTracingProgramID, ConverterProgramID};
}


