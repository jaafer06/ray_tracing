#pragma once
#include "Eigen/Dense"
#include "Eigen/Geometry" 

using namespace Eigen;

#define pi 3.14159265358979323846

template<typename T, unsigned int n, unsigned m>
std::istream& operator>>(std::istream& in, Matrix<T, n, m>& other)
{
    for (unsigned int i = 0; i < other.rows(); i++)
        for (unsigned int j = 0; j < other.cols(); j++)
            in >> other(i, j);
    return in;
}

inline Vector3f randomVector() {
    float  teta = ((float)rand() / RAND_MAX) * pi;
    float phi = ((float)rand() / RAND_MAX) * pi;

    // speed boost
    //float teta = uniformDistribution(gen) * pi;
    //float phi = uniformDistribution(gen) * pi;
    return { sin(teta) * cos(phi), sin(teta) * sin(phi), cos(phi) };
    //return Vector3f{ normalDistribution(gen), normalDistribution(gen), normalDistribution(gen) }.normalized();
}

inline Vector3f randomVector(const Vector3f& normal) {
    return normal * 1.001 + randomVector();
}

inline float clamp(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

inline Vector3f clamp(const Vector3f& vector, float min, float max) {
    return vector.unaryExpr([&min, &max](float c) { return clamp(c, min, max); });
}

inline Vector3f reflect(const Vector3f& incoming, const Vector3f& normal) {
    return incoming - 2 * normal.dot(incoming) * normal;
}