#pragma once
#include "Eigen/Dense"
#include "Eigen/Geometry" 
#include <time.h>
#include <stdlib.h>
#include <execution>
#include <algorithm>
#include <execution>
#include "cpuVersion/scene.h"
#include <random>
#include <atomic>

# define pi 3.14159265358979323846

using namespace Eigen;

static Vector3f light(1, 1, 1);


struct Color {
    float r;
    float g;
    float b;
    void operator=(const Vector3f& other) {
        r = other[0];
        g = other[1];
        b = other[2];
    }
};


class Camera {
public:

    Camera(void* buffer, unsigned int width, unsigned int height) : width(width), height(height) {
        srand((unsigned)time(NULL));

        position = Vector3f(0, 0, 6);
        focalLength = 1;
        up = Vector3f(0, 1, 0);
        right = Vector3f(1, 0, 0);
        this->buffer = static_cast<Color*>(buffer);

        worldWidth = width *  ( 1 / sqrt(width * height));
        worldHeight = height * ( 1 / sqrt(width * height));
        upper_left = Vector3f{ -worldWidth / 2, worldHeight / 2, -focalLength} + position;
        worldStep = worldWidth / width;

        scene.addSphere({ 0, 0, -4 }, 0.5);
        scene.addSphere({ 1.5, 0, -3 }, 0.5);
        scene.addSphere({0, -94, -40}, 100);

        scene.addCube({ 1, 1.5, -2 });
        //scene.addSphere({ 1, 1, -3 }, 0.5);

        gen = std::mt19937(rd());
        normalDistribution = std::normal_distribution<float>{ 0, 1 };
        uniformDistribution = std::uniform_real_distribution<float>{ 0,  pi};

    }

    void render() { 

        auto p = [this](Color& pixel) {
            int index = &pixel - buffer;
            int i = index % width;
            int j = index / width;
            Vector3f pixelWorldSpace = upper_left + worldStep * i * right - worldStep * j * up;
        
            renderPixel<2>(pixelWorldSpace, (height - 1 - j) * width + i);
        };

        std::for_each(std::execution::par_unseq, buffer, buffer+width*height-1, p);
        scene.cubes[0].rotate(0.1);
        //light += Vector3f(0, 0, -0.01);
        //upper_left += Vector3f{0, 0, -0.01};
    }

    template<unsigned int N>
    void renderPixel(Vector3f& pixelWorldSpace, unsigned int pixelIndex, unsigned int steps = 10) {

        float stepSize = worldStep / (float)(N + 1);
        constexpr unsigned int totalRays = N * N;
        Vector3f pixelColor{ 0, 0, 0 };
        Vector3f currentPosition = pixelWorldSpace;
        for (unsigned int i = 0; i < N; ++i) {
            currentPosition = pixelWorldSpace - up * stepSize * i;
            for (unsigned int j = 0; j < N; ++j) {
                currentPosition += right * stepSize * j;
                Ray ray(currentPosition, currentPosition - position);
                pixelColor += ray_color(ray, steps);
            }
        }

        buffer[pixelIndex] = (pixelColor / totalRays).cwiseSqrt();

    }

    Vector3f ray_color(const Ray& r, int depth) {
        HitRecord hitRecord;

        if (depth < 0) {
            return Vector3f(0, 0, 0);
        }

        if (scene.hit(r, 0.1, std::numeric_limits<float>::max(), hitRecord)) {
            Vector3f random = randomVector();
            Vector3f target = hitRecord.normal + (hitRecord.normal.dot(random) > 0 ? random : -random);
            return 0.4 * ray_color(Ray(hitRecord.p, target), depth - 1);
        }

        Vector3f unit_direction = r.direction();
        float t = 0.5 * (unit_direction[1] + 1.);
        return (1.0 - t) * Vector3f(1.0, 1.0, 1.0) + t * Vector3f(0.5, 0.7, 1.0);
    };


    inline Vector3f randomVector() {
        float  teta = ((float)rand() / RAND_MAX) * pi;
        float phi = ((float)rand() / RAND_MAX)*pi;

        // speed boost
        //float teta = uniformDistribution(gen) * pi;
        //float phi = uniformDistribution(gen) * pi;
        return  Vector3f{ sin(teta) * cos(phi), sin(teta) * sin(phi), cos(phi) };
        //return Vector3f{ normalDistribution(gen), normalDistribution(gen), normalDistribution(gen) }.normalized();
    }

private:
    Vector3f position;
    Vector3f up;
    Vector3f right;
    float focalLength;
    unsigned int width;
    unsigned int height;
    float worldWidth;
    float worldHeight;
    float worldStep;
    Vector3f upper_left;
    Color* buffer;
    Scene scene;
    std::random_device rd;
    std::mt19937 gen;
    std::normal_distribution<float> normalDistribution;
    std::uniform_real_distribution<float> uniformDistribution;

};
