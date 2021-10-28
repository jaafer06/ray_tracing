#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "Eigen/Dense"
#include <string>
#include <chrono>
#include <random>
#include "utils.h"
#include "gpu_version/ray_tracing_compute_shader.h"
#include "gpu_version/conversion_compute_shader.h"
#include <string>
#include "gpu_version/camera.h"
#include <vector>
#include <gpu_version/shape.h>

void GLAPIENTRY
MessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
        type, severity, message);
}
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static bool cameraMode = false;
constexpr static unsigned int width = 1080;
constexpr static unsigned int height = 720;
constexpr static unsigned int channels = 4;
constexpr static unsigned int pixelCount = width * height * channels;
static unsigned int number_of_samples = 1;

static Camera* globalCamera;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

int main()
{

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
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
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGL();
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    glViewport(0, 0, width, height);

    GLuint floatTexture;
    glGenTextures(1, &floatTexture);
    glBindTexture(GL_TEXTURE_2D, floatTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindImageTexture(2, floatTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
   
    GLuint fboId;
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, floatTexture, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    Camera camera{ width, height };
    globalCamera = &camera;
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);
    auto programIds = LoadShaders();
    RayTracingComputeShader computeShader{camera, programIds.ray_tracing};
    int timeLocation = glGetUniformLocation(programIds.ray_tracing, "time");
    computeShader.shapes.push_back(Box({ -1, 4.5, -3.5 }, { 1.5, 1, 1 }, Light{ { 12, 12, 12 } }));

    computeShader.shapes.push_back(Circle({ 1, 3, -10 }, 1, Lambertian{ { 0, 1, 0 } }));
    computeShader.shapes.push_back(Circle({ -1.5, 1.5, -4 }, 0.5, Metal{ { 1, 1, 1} }));
    computeShader.shapes.push_back(Circle({ 0, 1, -4 }, 0.5, Lambertian{ { 0.8, 0.8, 0} }));

    computeShader.shapes.push_back(Circle({ 1.5, 1, -3 }, 1.1, Lambertian{ {  0.5, 0, 0  } }));
    computeShader.shapes.push_back(Box({ 0, -50, 0}, {100, 1, 1}, Lambertian{ { 0.3, 0.9, 0.7 } }));
    computeShader.shapes.push_back(Box({ 3, 2, -4.5 }, { 1, 2, 1 }, Lambertian{ { 0, 1, 1 } }));
    computeShader.shapes.push_back(Circle({ -2, 1, -1 }, 0.5, Lambertian{ { 0.5, 1, 1} }));
    computeShader.shapes.push_back(Box({ -2, 1, -6 }, { 0.5, 0.5, 1 }, Lambertian{ { 1, 0, 0.5} }));

    computeShader.shapes.push_back(Triangle({ 0, 0, 0 }, { 1, 1, 0 }, { -1, 1, 0 }, Lambertian{ { 1, 0, 0} }));

    computeShader.updateShapeBuffer();
    unsigned int pixelBuffer;
    glGenBuffers(1, &pixelBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pixelBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * pixelCount, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, pixelBuffer);

    auto start = std::chrono::high_resolution_clock::now();

    glUseProgram(programIds.converter);
    int number_of_samples_location = glGetUniformLocation(programIds.converter, "n_samples");
    while (!glfwWindowShouldClose(window))
    {
        //std::cout << number_of_samples << std::endl;
        auto t1 = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::milli>(t1-start).count();
        glUseProgram(programIds.ray_tracing);
        glUniform1f(timeLocation, time/10000);

        computeShader.updateCameraBuffer();

        computeShader.compute(width, height, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        glUseProgram(programIds.converter);
        glUniform1ui(number_of_samples_location, number_of_samples);
        glDispatchCompute((GLuint)width, (GLuint)height, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        ++number_of_samples;
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glfwPollEvents();
        glfwSwapBuffers(window);
        auto t2 = std::chrono::high_resolution_clock::now();
        std:chrono::duration<double, std::milli> ms_double = t2 - t1;

        std::cout << ms_double.count() << "ms" << std::endl;

    }
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        cameraMode = !cameraMode;
        return;
    }  
    if (cameraMode) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            globalCamera->move(Camera::Direction::FORWARD);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            globalCamera->move(Camera::Direction::BACKWARD);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            globalCamera->move(Camera::Direction::LEFT);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            globalCamera->move(Camera::Direction::RIGHT);
        number_of_samples = 1;
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (cameraMode) {
        glfwSetCursorPos(window, width / 2, height / 2);
        float horizontalAngle = 0.002 * float(width / 2 - xpos);
        float verticalAngle = 0.002 * float(height / 2 - ypos);
        horizontalAngle = abs(horizontalAngle) > 0.01 ? horizontalAngle : 0;
        verticalAngle = abs(verticalAngle) > 0.01 ? verticalAngle : 0;
        globalCamera->rotate(horizontalAngle, verticalAngle);
        if (horizontalAngle != 0 || verticalAngle != 0) {
            number_of_samples = 1;
        }
    }

}