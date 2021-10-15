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

int main()
{
   /* std::vector<Shape> shapes;
    Material m = Lambertian{ {0, 1, 0} };
    Shape s = Circle({ 0.5, 0, 1 }, Lambertian{ { 0, 1, 0 } });
    std::cout << sizeof(Material) << std::endl;
    std::cout << sizeof(Circle) << std::endl;

    std::cout << sizeof(Shape) << std::endl;
    for (unsigned int index = 0; index < sizeof(Shape) / 4; ++index) {
        std::cout << reinterpret_cast<float*>(&s)[index] << std::endl;
    }*/
    constexpr unsigned int width = 720;
    constexpr unsigned int height = 480;
    constexpr unsigned int channels = 4;
    constexpr unsigned int size = width * height * channels;

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

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGL();
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    unsigned int* data = new unsigned int[size];
    float* datafloat = new float[size];

    GLuint floatTexture;
    glGenTextures(1, &floatTexture);
    glBindTexture(GL_TEXTURE_2D, floatTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindImageTexture(2, floatTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    GLuint redChannel;
    glGenTextures(1, &redChannel);
    glBindTexture(GL_TEXTURE_2D, redChannel);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, width, height, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindImageTexture(5, redChannel, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);


    GLuint greenChannel;
    glGenTextures(1, &greenChannel);
    glBindTexture(GL_TEXTURE_2D, greenChannel);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, width, height, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindImageTexture(6, greenChannel, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

    GLuint blueChannel;
    glGenTextures(1, &blueChannel);
    glBindTexture(GL_TEXTURE_2D, blueChannel);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, width, height, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindImageTexture(7, blueChannel, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
   
    GLuint fboId;
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    // attach the texture to FBO color attachment point
    glFramebufferTexture2D(GL_FRAMEBUFFER,       // 1. fbo target: GL_FRAMEBUFFER
        GL_COLOR_ATTACHMENT0, // 2. attachment point
        GL_TEXTURE_2D,
        floatTexture,            // 4. tex ID
        0);                   // 5. mipmap level: 0(base)

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId); // src FBO (multi-sample)
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);     // dst FBO (single-sample)

    Camera camera{ width, height };
    auto programIds = LoadShaders();
    RayTracingComputeShader computeShader{camera, programIds.ray_tracing};
    computeShader.shapes.push_back(Circle({ 0.5, 0, 1 }, Lambertian{ { 0, 1, 0 } }));
    computeShader.shapes.push_back(Circle({ 0.5, 0, 1 }, Lambertian{ { 0, 1, 0 } }));
    computeShader.updateShapeBuffer();
    
    //--------

    unsigned int* textureData = new unsigned int[size];
    for (unsigned int i = 0; i < size; ++i) {
        textureData[i] = 100;
    }
    unsigned int textureBuffer;
    glGenBuffers(1, &textureBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * size, textureData, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, textureBuffer);


    while (!glfwWindowShouldClose(window))
    {

        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::chrono::duration;
        using std::chrono::milliseconds;
        auto t1 = high_resolution_clock::now();

        //camera.move(Camera::Direction::FORWARD);
        //computeShader.updateCameraBuffer();

        glClearTexImage(redChannel, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, NULL);
        glClearTexImage(greenChannel, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, NULL);
        glClearTexImage(blueChannel, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, NULL);

        computeShader.compute(width, height, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        glUseProgram(programIds.converter);
        glDispatchCompute((GLuint)width, (GLuint)height, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        glBlitFramebuffer(0, 0, width, height,         // src rect
            0, 0, width, height,         // dst rect
            GL_COLOR_BUFFER_BIT,         // buffer mask
            GL_LINEAR);                  // scale filter


        glfwPollEvents();
        glfwSwapBuffers(window);
        auto t2 = high_resolution_clock::now();
        duration<double, std::milli> ms_double = t2 - t1;

        std::cout << ms_double.count() << "ms" << std::endl;

    }
    return 0;
}
