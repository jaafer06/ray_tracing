#include "Eigen/Dense"
#include <execution>
#include <algorithm>
#include <execution>

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

class ray {
    public:
        ray() {}
        ray(const Vector3f& origin, const Vector3f& direction)
            : orig(origin), dir(direction)
        {}

        Vector3f origin() const  { return orig; }
        Vector3f direction() const { return dir; }

        Vector3f at(double t) const {
            return orig + t*dir;
        }

    public:
        Vector3f orig;
        Vector3f dir;
};

struct hit_record {
    Vector3f p;
    Vector3f normal;
    double t;
};

class hittable {
    public:
        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
};

inline float hit_sphere(const Vector3f& center, double radius, const ray& r) {
    Vector3f oc = r.origin() - center;
    auto a = r.direction().dot(r.direction());
    auto b = 2.0 * oc.dot(r.direction());
    auto c = oc.dot(oc) - radius*radius;
    auto discriminant = b*b - 4*a*c;
    if (discriminant < 0) {
        return -1.0;
    } else {
        return (-b - sqrt(b*b-4*a *c) ) / 2*a;
    }
}

inline Vector3f sphereNormal(const Vector3f& center, const Vector3f& at) {
    return (at - center ).normalized();
}

Vector3f ray_color(const ray& r) {

    float t = hit_sphere(Vector3f(0,0,-2), 0.5, r);
    if (t >= 0.0) {
        Vector3f normal = sphereNormal(Vector3f(0,0,-3), r.at(t));
        Vector3f surfaceToLight = (light - r.at(t)).normalized();
        float intensity = surfaceToLight.dot(normal);
        return  0.7*(Vector3f(1, 0, 0)* std::max(intensity, 0.f)) + 0.3 * Vector3f(1, 0, 0);
    }

    Vector3f unit_direction = r.direction().normalized();
    t = 0.5*(unit_direction[1] + 1);
    return (1.0 - t) * Vector3f(1.0, 1.0, 1.0) + t * Vector3f(0.5, 0.7, 1.0);
}

class Camera {
public:

    Camera(void* buffer, unsigned int width, unsigned int height): width(width), height(height) {
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
    }

    void render() { 
        
        auto p = [this](Color& pixel) {
            int index = &pixel - buffer;
            int i = index % width;
            int j = index / width;
            Vector3f pixelWorldSpace = upper_left + (du * i + du/2) * right - (dv * j + dv/2) * up ;
            ray r(position, pixelWorldSpace - position);
            Vector3f pixel_color = ray_color(r);
            buffer[(height-1-j)*width+i] = pixel_color;
        };

        std::for_each(std::execution::par, buffer, buffer+width*height-1, p);

         //for (int j = 0; j < height; ++j) {
         //    for (int i = 0; i < width; ++i) {
         //        Vector3f pixelWorldSpace = upper_left + (du * i + du/2) * right - (dv * j + dv/2) * up ;
         //        ray r(position, pixelWorldSpace - position);
         //        Vector3f pixel_color = ray_color(r);
         //        buffer[(height-1-j)*width+i] = pixel_color;
         //    }
         //}
        light += Vector3f(0, 0, -0.01);
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
};


