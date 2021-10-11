#version 430
#define size 15
layout(local_size_x = size, local_size_y = size, local_size_z=1) in;

layout(binding = 5, r32ui) uniform uimage2D red;
layout(binding = 6, r32ui) uniform uimage2D green;
layout(binding = 7, r32ui) uniform uimage2D blue;

uniform vec3 cameraPosition;
uniform vec3 cameraDirection;
uniform vec3 upperLeft;
uniform vec3 up;
uniform vec3 right;
uniform float worldStep;

struct Ray {
	vec3 origin;
	vec3 direction;
};

struct Sphere {
	vec3 center;
	float radius;
	vec3 color;
};

bool hitSphere(in Ray r, in Sphere sphere) {
	vec3 oc = r.origin - sphere.center;
	float b = dot(r.direction, oc);
	float c = dot(oc, oc) - sphere.radius * sphere.radius;
	float discriminant = b * b - c;
	return discriminant > 0;
};


vec3 ray_color(in Ray ray, in Sphere[2] spheres, uint depth) {
	for (uint index = 0; index < spheres.length(); ++index) {
		if (hitSphere(ray, spheres[index])) {
			return spheres[index].color;
		}
	}
	float t = (upperLeft.y - ray.origin.y);
	return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);

};

void main() {
	Sphere s1 = Sphere(vec3(0, 0, -4), 1, vec3(1, 0, 0));
	Sphere s2 = Sphere(vec3(0, 94, -40), 100, vec3(0.3, 0.9, 0.7));

	Sphere[2] spheres;
	spheres[0] = s1;
	spheres[1] = s2;

	vec3 pixelCoordinate = upperLeft + worldStep * gl_WorkGroupID.x * right - worldStep * gl_WorkGroupID.y * up;
	float gridStep = worldStep / (size+1);
	vec3 rayOrigin = pixelCoordinate + gridStep * (gl_LocalInvocationID.x+1) * right - gridStep * (gl_LocalInvocationID.y+1) * up;
	vec3 rayDirection = normalize(rayOrigin - cameraPosition);
	Ray ray = Ray(rayOrigin, rayDirection);

	vec3 colorf = ray_color(ray, spheres, 0);
	uvec4 color = uvec4(255 * vec4(colorf, 1.));

	imageAtomicAdd(red, ivec2(gl_WorkGroupID.xy), color.x);
	imageAtomicAdd(green, ivec2(gl_WorkGroupID.xy), color.y);
	imageAtomicAdd(blue, ivec2(gl_WorkGroupID.xy), color.z);

}