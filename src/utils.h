#pragma once
#include "Eigen/Dense"
#include "Eigen/Geometry" 

using namespace Eigen;

#define pi 3.14159265358979323846


inline Vector3f randomVector() {
    float  teta = ((float)rand() / RAND_MAX) * pi;
    float phi = ((float)rand() / RAND_MAX) * pi;

    // speed boost
    //float teta = uniformDistribution(gen) * pi;
    //float phi = uniformDistribution(gen) * pi;
    return { sin(teta) * cos(phi), sin(teta) * sin(phi), cos(phi) };
    //return Vector3f{ normalDistribution(gen), normalDistribution(gen), normalDistribution(gen) }.normalized();
}

inline Vector3f randomVector(Vector3f normal) {
    return normal * 1.01 + randomVector();
}