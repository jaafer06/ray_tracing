#include "Eigen/Dense"

using namespace Eigen;

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

inline bool hit_sphere(const Vector3f& center, double radius, const ray& r) {
    Vector3f oc = r.origin() - center;
    auto a = r.direction().dot(r.direction());
    auto b = 2.0 * oc.dot(r.direction());
    auto c = oc.dot(oc) - radius*radius;
    auto discriminant = b*b - 4*a*c;
    return (discriminant > 0);
}

Vector3f ray_color(const ray& r) {
     if (hit_sphere(Vector3f(0,0,-3), 0.5, r))
        return Vector3f(1, 0, 0);
    Vector3f unit_direction = r.direction().normalized();
    float t = 0.5*(unit_direction[1] + 1);
    return (1.0-t) * Vector3f(1.0, 1.0, 1.0) + t*Vector3f(0.5, 0.7, 1.0);
}

class Camera {
public:
    Camera(Vector3f position, Vector3f up, Vector3f right, float focalLength) 
    : position(position), focalLength(focalLength), up(up), right(right) 
    {

    }

    Camera(float* buffer, unsigned int width, unsigned int height): width(width), height(height), buffer(buffer) {
        position = Vector3f(0, 0, 0);
        focalLength = 1;
        up = Vector3f(0, 1, 0);
        right = Vector3f(1, 0, 0);
    }

    void render() { 
        
        float worldWidth = width * 1/sqrt(width*height);
        float worldHeight = height * 1/sqrt(width*height);
        Vector3f upper_left(-worldWidth/2, worldHeight/2, 1);


        float du = worldWidth/(width-1);
        float dv = worldHeight/(height-1);

        for (int j = 0; j < height; ++j) {
            for (int i = 0; i < width; ++i) {
                Vector3f pixelWorldSpace = upper_left + (du * i + du/2) * right - (dv * j + dv/2) * up ;
                ray r(position, pixelWorldSpace - position);
                Vector3f pixel_color = ray_color(r);
                buffer[(height-1-j)*width*3+i*3] = pixel_color[0];
                buffer[(height-1-j)*width*3+i*3+1] = pixel_color[1];
                buffer[(height-1-j)*width*3+i*3+2] = pixel_color[2];
            }

        }
    }

private:
    Vector3f position;
    Vector3f up;
    Vector3f right;
    float focalLength;
    unsigned int width;
    unsigned int height;
    float* buffer;
};





