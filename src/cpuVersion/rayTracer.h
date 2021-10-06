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
#include "rayMarching.h"

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

    enum class Direction {
        FORWARD, BACKWARD, RIGHT, LEFT
    };

    Camera(void* buffer, unsigned int width, unsigned int height) : width(width), height(height) {
        srand((unsigned)time(NULL));

        position = Vector3f(1, 0, 5);
        focalLength = 1;
        up = Vector3f(0, 1, 0);
        right = Vector3f(1, 0, 0);
        lookingAt = Vector3f(0, 0, -1);
        this->buffer = static_cast<Color*>(buffer);

        worldWidth = width *  ( 1 / sqrt(width * height));
        worldHeight = height * ( 1 / sqrt(width * height));
        upper_left = Vector3f{ -worldWidth / 2, worldHeight / 2, -focalLength} + position;
        worldStep = worldWidth / width;
        materials.push_back(RayMarching::Lambertian{ {0.8, 0.8, 0} });
        materials.push_back(RayMarching::Lambertian{ {0.5, 0, 0} });
        materials.push_back(RayMarching::Lambertian{ {0.3, 0.9, 0.7} });
        materials.push_back(RayMarching::Light{ {5, 5, 5} });
        materials.push_back(RayMarching::Metal{ {0.3, 0.7, 0.7} });

        // -----------
        materials.push_back(RayMarching::DebugMaterial{ {0.8, 0.8, 0} });
        //scene.push_back(RayMarching::Box{ 0, {0, 0, 0}, {0.5, 0.5, 0.5}, 1 });
        // -----------
    
         // test
        scene.push_back(RayMarching::Sphere{ 2, {0, 0, 0}, 0.5 });
        scene.push_back(RayMarching::Sphere{ 1, {0, 0, -4}, 0.5 });
        scene.push_back(RayMarching::Box{ 4, {1.5, 1.5, -7}, {0, 0, 0}, 3  });
        scene.push_back(RayMarching::Sphere{ 3, { 1, 2, -2 }, 1 });
        

        scene.push_back(RayMarching::Sphere{ 2, { 0, -94, -40}, 100 });

    }
    
    void rotate(float horizontal, float vertical) {
        const auto rotation = (AngleAxisf(horizontal, up) * AngleAxisf(vertical, right)).toRotationMatrix();
        up = rotation * up;
        right = rotation * right;
        lookingAt = rotation * lookingAt;
        upper_left = (-worldWidth / 2 * right)+ (worldHeight / 2 * up) + lookingAt * focalLength + position;

    }

    void move(Direction direction) {
        switch (direction)
        {
        case Direction::FORWARD:
            position += lookingAt * 0.2;
            break;
        case Direction::BACKWARD:
            position -= lookingAt * 0.2;
            break;
        case Direction::RIGHT:
            position += right * 0.2;
            break;
        case Direction::LEFT:
            position -= right * 0.2;
            break;
        default:
            break;
        }
        upper_left = (-worldWidth / 2 * right)+ (worldHeight / 2 * up) + lookingAt * focalLength + position;
    }

    void render() { 

        const auto p = [this](Color& pixel) {
            int index = &pixel - buffer;
            int i = index % width;
            int j = index / width;
            Vector3f pixelWorldSpace = upper_left + worldStep * i * right - worldStep * j * up;
            renderPixel<2>(pixelWorldSpace, (height - 1 - j) * width + i, 10);
        };

        std::for_each(std::execution::par_unseq, buffer, buffer+width*height-1, p);
        

        //light += Vector3f(0, 0, -0.01);
        //upper_left += Vector3f{0, 0, -0.01};
        //reinterpret_cast<RayMarching::Box*>(&scene[0])->addR();

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
                RayMarching::Ray ray{ currentPosition, (currentPosition - position).normalized(), 0 };
                pixelColor += ray_color(ray, steps);
            }
        }

        buffer[pixelIndex] = (static_cast<Vector3f>(pixelColor / totalRays)).cwiseSqrt();
        //buffer[pixelIndex] = clamp(static_cast<Vector3f>(pixelColor / totalRays), 0.f, 1.f).cwiseSqrt();

    }

    Vector3f ray_color(RayMarching::Ray& ray, int depth) {
        Vector3f color;
        if (depth < 0) {
            return Vector3f(0, 0, 0);
        }

        //if (scene.hit(r, 0.001, std::numeric_limits<float>::max(), hitRecord)) {
        //    Ray scattered;
        //    Vector3f attenuation;
        //    if (hitRecord.material->scatter(r, hitRecord, attenuation, scattered)) {
        //        return attenuation.cwiseProduct(ray_color(scattered, depth - 1));

        //    } else {
        //        return  hitRecord.material->emit();
        //    }
        //}
        auto hitResult = RayMarching::getColosestShape(scene, ray);
        if (hitResult.distance != RayMarching::inf) {
            ray.t = hitResult.distance;
            const unsigned int materialIndex = std::visit(RayMarching::Visitors::MaterialIndex{}, hitResult.shape);
            const bool scattered = std::visit(RayMarching::Visitors::Scatter{ray, color, hitResult.shape }, materials[materialIndex]);
            if (scattered) {
                return color.cwiseProduct(ray_color(ray, depth - 1));
            }
            return color;
        }
        const Vector3f& unit_direction = ray.direction;
        float t = 0.5 * (unit_direction[1] + 1.);
        return (1.0 - t) * Vector3f(1.0, 1.0, 1.0) + t * Vector3f(0.5, 0.7, 1.0);
        //return { 0, 0, 0 };
    };

private:
    Vector3f position;
    Vector3f up;
    Vector3f right;
    Vector3f lookingAt;
    float focalLength;
    unsigned int width;
    unsigned int height;
    float worldWidth;
    float worldHeight;
    float worldStep;
    Vector3f upper_left;
    Color* buffer;
    std::vector<RayMarching::BasicShape> scene;
    std::vector<RayMarching::Materials> materials;

};
