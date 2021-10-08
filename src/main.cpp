#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "Eigen/Dense"
#include <string>
#include "cpuVersion/rayTracer.h"
#include <chrono>
#include <random>
#include "utils.h"
#include "shaders/loadShader.h"
#include <string>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

int main()
{

    unsigned int width = 500;
    unsigned int height = 500;
    unsigned int channels = 4;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(width, height, "Ray tracing", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGL();
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    unsigned int size = width * height * channels;
    float* data = new float[size];
    for (unsigned int index = 0; index < size; ++index) {
        data[index] = 0;
    }

    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(float), data, GL_DYNAMIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);

    // ---------------
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, data);


    GLuint fboId;
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    // attach the texture to FBO color attachment point
    glFramebufferTexture2D(GL_FRAMEBUFFER,       // 1. fbo target: GL_FRAMEBUFFER
        GL_COLOR_ATTACHMENT0, // 2. attachment point
        GL_TEXTURE_2D,        // 3. tex target: GL_TEXTURE_2D
        textureId,            // 4. tex ID
        0);                   // 5. mipmap level: 0(base)
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "error ma dude" << std::endl;

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId); // src FBO (multi-sample)
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);     // dst FBO (single-sample)

    // ------
    GLuint shaderId = LoadShaders(GL_COMPUTE_SHADER, "../src/shaders/RayTracing.shader");
    glUseProgram(shaderId);
    glDispatchCompute((GLuint)width, (GLuint)height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    data = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
    
    //for (unsigned int index = 0; index < size; index += 4) {
    //    std::cout << data[index] << " " << data[index + 1] << " " << data[index + 2] << " " << data[index + 3] << std::endl;

    //    //data[index] = 1;
    //}


    while (!glfwWindowShouldClose(window))
    {
        std::cout << data[0] << std::endl;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, data);
        glBlitFramebuffer(0, 0, width, height,         // src rect
            0, 0, width, height,         // dst rect
            GL_COLOR_BUFFER_BIT,         // buffer mask
            GL_LINEAR);                  // scale filter

        glDispatchCompute((GLuint)width, (GLuint)height, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::chrono::duration;
        using std::chrono::milliseconds;
        auto t1 = high_resolution_clock::now();

        //camera.render();

        auto t2 = high_resolution_clock::now();
        duration<double, std::milli> ms_double = t2 - t1;

        //std::cout << ms_double.count() << "ms" << std::endl;

        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    return 0;
}
