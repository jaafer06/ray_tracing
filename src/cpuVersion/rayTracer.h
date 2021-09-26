#pragma once
#include "Eigen/Dense"
#include <execution>
#include <algorithm>
#include <execution>
#include "cpuVersion/scene.h"
#include <random>
#include <atomic>

using namespace Eigen;

static Vector3f light(1, 1, 1);

template<unsigned int N, unsigned int M>
Matrix<float, N* M, 3> grid() {
    Matrix<float, N* M, 3, RowMajor> result;
    float stepSizeX = 1 / (float)(M - 1);
    float stepSizey = 1 / (float)(N - 1);

    for (unsigned int i = 0; i < N; ++i) {
        for (unsigned int j = 0; j < M; ++j) {
            result.row(i*N + j) = Vector3f{ j * stepSizeX, i * stepSizey, 0 };
        }
    }
    return result;
}

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
        position = Vector3f(0, 0, 0);
        focalLength = 1;
        up = Vector3f(0, 1, 0);
        right = Vector3f(1, 0, 0);
        this->buffer = static_cast<Color*>(buffer);

        worldWidth = width * 1 / sqrt(width * height);
        worldHeight = height * 1 / sqrt(width * height);
        upper_left = { -worldWidth / 2, worldHeight / 2, -1 };
        du = worldWidth / (width - 1);
        dv = worldHeight / (height - 1);

        scene.addSphere({ 0, 0, -4 }, 0.5);
        //scene.addSphere({ 2, 0, -3 }, 0.5);
        scene.addSphere({0, -100, -40}, 100);


        gen = std::mt19937(rd()); // Standard mersenne_twister_engine seeded with rd()
        d = std::normal_distribution<>{ 0, 1 };

    }

    void render() { 
        
        auto p = [this](Color& pixel) {
            int index = &pixel - buffer;
            int i = index % width;
            int j = index / width;
            //vector3f pixelWorldSpace = upper_left + (du * i + du / 2) * right - (dv * j + dv / 2) * up;
            //todo
            Vector3f pixelWorldSpace = upper_left + du * i  * right - dv * j * up;

            //Ray ray(pixelWorldSpace, pixelWorldSpace - position);
            //Vector3f pixel_color = ray_color(ray, 10);

            constexpr unsigned int size = 5*5;
            Matrix<float, size, 3, RowMajor> pixels = grid<5, 5>();
            pixels.block<size, 1>(0, 0) *= du;
            pixels.block<size, 1>(0, 1) *= dv;
            pixels.rowwise() += pixelWorldSpace.transpose();
            
            Matrix<float, size, 3> pixel_colors;
            Vector3f* start = static_cast<Vector3f*>((void*)pixels.data());
            Vector3f* end = start + size;
            auto p = [&pixel_colors, this, start](Vector3f& pixelPosition) {
                unsigned int index = &pixelPosition - start;
                Ray ray(pixelPosition, pixelPosition - position);
                pixel_colors.row(index) = ray_color(ray, 10);
            };

            std::for_each(std::execution::par, start, end, p);
            buffer[(height - 1 - j) * width + i] = pixel_colors.colwise().mean();
        };

        std::for_each(std::execution::par, buffer, buffer+width*height-1, p);

         //for (int j = 0; j < height; ++j) {
         //    for (int i = 0; i < width; ++i) {
         //        Vector3f pixelWorldSpace = upper_left + (du * i + du/2) * right - (dv * j + dv/2) * up ;
         //        Ray r(position, pixelWorldSpace - position);
         //        Vector3f pixel_color = ray_color(r, 5);
         //        buffer[(height-1-j)*width+i] = pixel_color;
         //    }
         //}
        light += Vector3f(0, 0, -0.01);
    }

    Vector3f ray_color(const Ray& r, int depth) {
        HitRecord hitRecord;

        if (depth < 0) {
            return Vector3f(0, 0, 0);
        }

        if (scene.hit(r, 0.1, std::numeric_limits<float>::max(), hitRecord)) {
            //Vector3f surfaceToLight = (light - hitRecord.p).normalized();
            //float intensity = surfaceToLight.dot(hitRecord.normal);
            //return  0.7 * (Vector3f(1, 0, 0) * std::max(intensity, 0.f)) + 0.3 * Vector3f(1, 0, 0);
            //Vector3f target = hitRecord.p + hitRecord.normal + randomVector();
            Vector3f target = hitRecord.p + hitRecord.normal + randomVector();
            return 0.5 * ray_color(Ray(hitRecord.p, target - hitRecord.p), depth - 1);
        }

        Vector3f unit_direction = r.direction();
        float t = 0.5 * (unit_direction[1] + 1.);
        return (1.0 - t) * Vector3f(1.0, 1.0, 1.0) + t * Vector3f(0.5, 0.7, 1.0);
    };


    inline Vector3f randomVector() {
        return Vector3f{ float(d(gen)), float(d(gen)), float(d(gen)) }.normalized();
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
    Vector3f upper_left;
    float du, dv;
    Color* buffer;
    Scene scene;
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen; // Standard mersenne_twister_engine seeded with rd()
    std::normal_distribution<> d;
    

};
