#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "Eigen/Dense"
#include <string>
#include "cpuVersion/rayTracer.h"
#include <chrono>
#include <random>

static void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}

template<typename T, unsigned int n, unsigned m>
std::istream& operator>>(std::istream& in, Matrix<T, n, m>& other)
{
    for (unsigned int i = 0; i < other.rows(); i++)
        for (unsigned int j = 0; j < other.cols(); j++)
            in >> other(i, j);
    return in;
}

int main()
{

    /*auto m = grid<3, 3>();
    std::cout << m.colwise().mean() << std::endl;*/
    //std::for_each(std::execution::par, 0, 10, [](int i) {std::cout << "i" << std::endl; });

    //std::cout << m << std::endl;
    //std::cout << "--------------" << std::endl;

    //m.block<9, 1>(0, 0) *= 5;
    //m.block<9, 1>(0, 1) *= 10;

    //std::cout << m << std::endl;
    unsigned int width = 960;
    unsigned int height = 540;
    unsigned int channels = 3;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(width, height, "Ray tracing", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGL(glfwGetProcAddress);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    unsigned int size = width * height * channels;
    float *data = new float[size];
    Camera camera(data, width, height);

    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, data);

    // create a renderbuffer object to store depth info
    GLuint rboId;
    glGenRenderbuffers(1, &rboId);
    glBindRenderbuffer(GL_RENDERBUFFER, rboId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    // create a framebuffer object
    GLuint fboId;
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    // attach the texture to FBO color attachment point
    glFramebufferTexture2D(GL_FRAMEBUFFER,       // 1. fbo target: GL_FRAMEBUFFER
                           GL_COLOR_ATTACHMENT0, // 2. attachment point
                           GL_TEXTURE_2D,        // 3. tex target: GL_TEXTURE_2D
                           textureId,            // 4. tex ID
                           0);                   // 5. mipmap level: 0(base)

    // attach the renderbuffer to depth attachment point
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,      // 1. fbo target: GL_FRAMEBUFFER
                              GL_DEPTH_ATTACHMENT, // 2. attachment point
                              GL_RENDERBUFFER,     // 3. rbo target: GL_RENDERBUFFER
                              rboId);              // 4. rbo ID

    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "error ma dude" << std::endl;

    // switch back to window-system-provided framebuffer

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId); // src FBO (multi-sample)
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);     // dst FBO (single-sample)
    glBlitFramebuffer(0, 0, width, height,         // src rect
                      0, 0, width, height,         // dst rect
                      GL_COLOR_BUFFER_BIT,         // buffer mask
                      GL_LINEAR);                  // scale filter

    glfwSwapBuffers(window);
    camera.render();
    while (!glfwWindowShouldClose(window))
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, data);
        glBlitFramebuffer(0, 0, width, height,         // src rect
                      0, 0, width, height,         // dst rect
                      GL_COLOR_BUFFER_BIT,         // buffer mask
                      GL_LINEAR);                  // scale filter
        
        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::chrono::duration;
        using std::chrono::milliseconds;

        auto t1 = high_resolution_clock::now();

        camera.render();
        auto t2 = high_resolution_clock::now();

    /* Getting number of milliseconds as an integer. */
        auto ms_int = duration_cast<milliseconds>(t2 - t1);
        
        /* Getting number of milliseconds as a double. */
        duration<double, std::milli> ms_double = t2 - t1;

        std::cout << ms_double.count() << "ms" << std::endl;

        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    return 0;
}