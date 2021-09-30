#pragma once
#include <vector>
#include <limits>
#include "Eigen/Dense"
#include "utils.h"

using namespace Eigen;

namespace Materials {
    
}


struct HitRecord {
    Vector3f p;
    Vector3f normal;
    double t;
};


class Ray {
public:
    Ray() {}
    Ray(const Vector3f& origin, const Vector3f& direction)
        : orig(origin), dir(direction.normalized())
    {};

    Vector3f origin() const { return orig; };
    Vector3f direction() const { return dir; };

    Vector3f at(double t) const {
        return orig + t * dir;
    };

public:
    Vector3f orig;
    Vector3f dir;
};


class Sphere {
public:
    Sphere(Vector3f center, float radius) : center(center), radius(radius) {}
    inline bool hit(const Ray& r, HitRecord& rec) const {
        Vector3f oc = r.origin() - center;
        float b = oc.dot(r.direction());
        float c = oc.dot(oc) - radius * radius;
        float discriminant = b * b - c;
        if (discriminant < 0) {
            return false;
        } else {
            rec.t = -b - sqrt(discriminant);
            rec.p = r.at(rec.t);
            rec.normal = normalAt(rec.p);
            return true;
        }
    };

    inline Vector3f normalAt(Vector3f& point) const {
        return (point - center).normalized();
    }

private:
    Vector3f center;
    float radius;
};



class Cube {
public:
    Cube(Vector3f center) : center(center) {
        normals << 0, 0, -1,
                0, 0, 1,
                0, 1, 0,
                0, -1, 0,
                1, 0, 0,
                -1, 0, 0;
        betas = Matrix<float, 6, 1>::Ones() * 0.5;
        rotation = Matrix3f::Identity();
    }

    inline bool hit(const Ray& ray, HitRecord& rec) const {
        Vector3f originTransformed = rotation*(ray.orig - center);
        Vector3f directionTransformed = rotation*ray.dir;
        Matrix<float, 6, 1> ts =  (betas - normals * originTransformed).cwiseQuotient(normals *directionTransformed);
        Matrix<float, 6, 3> intersectionPoints = (ts * directionTransformed.transpose()).rowwise() + originTransformed.transpose();
        //std::cout << intersectionPoint.cwiseAbs().maxCoeff()  << std::endl;
        
        Matrix<float, 6, 1> m = intersectionPoints.cwiseAbs().rowwise().maxCoeff();
        auto check = (m.array() == 0.5);
        if (check.any()) {
            float tmin = std::numeric_limits<float>::max();
            unsigned int normalIndex = 0;

            for (int index = 0; index < 6; ++index) {

                if (m(index) == 0.5 && tmin > ts(index)) {
                    tmin = ts(index);
                    normalIndex = 0;
                }
            };
            rec.normal = normals.row(normalIndex);
            rec.p = rotation.transpose() * intersectionPoints.row(normalIndex).transpose() + center;
            rec.t = ts(normalIndex);
            return true;
        }
        return false;

    };

    void rotate(float angle) {
        rotation *= (AngleAxisf(angle * pi, Vector3f::UnitX()) * AngleAxisf(angle * pi, Vector3f::UnitY())).toRotationMatrix();
    };

private:
    Vector3f center;
    Matrix<float, 6, 3> normals;
    Matrix<float, 6, 1> betas;
    Matrix3f rotation;
};


class Scene {
public:
    Scene() {}

    void addSphere(Vector3f center, float radius) {
        spheres.push_back(Sphere(center, radius));
    }

    void addCube(Vector3f center) {
        cubes.push_back(center);
    }

    bool hit(const Ray& r, double t_min, double t_max, HitRecord& hitRecord) const {
        float minDistance = std::numeric_limits<float>::max();
        bool hitSomething = false;
        HitRecord temp;
        for (const Sphere& sphere : spheres) {
            if (sphere.hit(r, temp) && temp.t < minDistance && t_min < temp.t && temp.t < t_max) {
                minDistance = temp.t;
                hitSomething = true;
                hitRecord = temp;
            }
        }

        for (const Cube& cubes: cubes) {
            if (cubes.hit(r, temp) && temp.t < minDistance && t_min < temp.t && temp.t < t_max) {
                minDistance = temp.t;
                hitSomething = true;
                hitRecord = temp;
            }
        }

        return hitSomething;
    };


public:
    std::vector<Sphere> spheres;
    std::vector<Cube> cubes;
};