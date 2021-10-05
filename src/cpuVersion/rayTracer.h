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
#include "utils.h"

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

        position = Vector3f(1, 0, 6);
        focalLength = 1;
        up = Vector3f(0, 1, 0);
        right = Vector3f(1, 0, 0);
        this->buffer = static_cast<Color*>(buffer);

        worldWidth = width *  ( 1 / sqrt(width * height));
        worldHeight = height * ( 1 / sqrt(width * height));
        upper_left = Vector3f{ -worldWidth / 2, worldHeight / 2, -focalLength} + position;
        worldStep = worldWidth / width;

        scene.addSphere({ 0, 0, -4 }, 0.5, new Lambertian({0.8, 0.8, 0}));
        scene.addSphere({ 1.5, 0, -3 }, 0.5, new Lambertian({0.5, 0, 0}));
        scene.addSphere({ 0, -94, -40 }, 100, new Lambertian({0.3, 0.9, 0.7}));

        scene.addCube({ 1, 2, -2 }, new LightSource({5, 5, 5}));
        //scene.addSphere({ 1, 2, -2 }, 0.5, new LightSource({ 5, 5, 5 }));

        //scene.addCube({ 1, 1.5, -2 }, new Lambertian({ 0.8, 0, 0.8 }));

        scene.addSphere({ 4.5, 1, -3 }, 1, new Metal({ 0.8, 0.5, 1 }, 0.08));
        //scene.addSphere({ 4.5, 1, -3 }, 1, new Lambertian({ 0.8, 0.5, 1 }));
        scene.addCube({ -1, 0, -2 }, new Lambertian({ 0.5, 0.5, 1 }));


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
            renderPixel<15>(pixelWorldSpace, (height - 1 - j) * width + i, 10);
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

        buffer[pixelIndex] = (static_cast<Vector3f>(pixelColor / totalRays)).cwiseSqrt();
        //buffer[pixelIndex] = clamp(static_cast<Vector3f>(pixelColor / totalRays), 0.f, 1.f).cwiseSqrt();

    }

    Vector3f ray_color(const Ray& r, int depth) {
        HitRecord hitRecord;

        if (depth < 0) {
            return Vector3f(0, 0, 0);
        }

        if (scene.hit(r, 0.001, std::numeric_limits<float>::max(), hitRecord)) {
            Ray scattered;
            Vector3f attenuation;
            if (hitRecord.material->scatter(r, hitRecord, attenuation, scattered)) {
                return attenuation.cwiseProduct(ray_color(scattered, depth - 1));

            } else {
                return  hitRecord.material->emit();
            }
        }

        //Vector3f unit_direction = r.direction();
        //float t = 0.5 * (unit_direction[1] + 1.);
        //return (1.0 - t) * Vector3f(1.0, 1.0, 1.0) + t * Vector3f(0.5, 0.7, 1.0);
        return { 0., 0., 0. };
    };

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
