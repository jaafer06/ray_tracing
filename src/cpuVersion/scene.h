#pragma once
#include <vector>
#include <limits>
#include "Eigen/Dense"

using namespace Eigen;

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


class Scene {
public:
    Scene() {}

    void addSphere(Vector3f center, float radius) {
        spheres.push_back(Sphere(center, radius));
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

        return hitSomething;
    };


private:
    std::vector<Sphere> spheres;
};