#version 430
#define size 15
layout(local_size_x = size, local_size_y = size, local_size_z=1) in;

layout(binding = 5, r32ui) uniform uimage2D red;
layout(binding = 6, r32ui) uniform uimage2D green;
layout(binding = 7, r32ui) uniform uimage2D blue;


layout(std430, binding = 2) buffer pixelBuffer
{
	uint pixels[];
};

layout(std430, binding = 1) buffer cameraBuffer
{
	vec3 cameraPosition;
	vec3 cameraDirection;
	vec3 upperLeft;
	vec3 up;
	vec3 right;
	float worldStep;
};

struct Material {
	vec3 color;
	uint type;
	float[4] data;
};

struct Shape {
	mat4 transformation;
	uint type;
	float[11] data;
	Material material;
};

uniform uint shapeCount;
layout(std430, binding = 0) buffer shapeBuffer
{
	Shape shapes[];
};

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

//vec3 hit(in Shape shape, in Ray ray) {
//
//}


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
	//imageAtomicAdd(red, ivec2(gl_WorkGroupID.xy), color.x);
	//imageAtomicAdd(green, ivec2(gl_WorkGroupID.xy), color.y);
	//imageAtomicAdd(blue, ivec2(gl_WorkGroupID.xy), color.z);

	uint index = 4 *gl_WorkGroupID.x + 4 * gl_NumWorkGroups.x * gl_WorkGroupID.y;
	atomicAdd(pixels[index], color.x);
	atomicAdd(pixels[index+1], color.y);
	atomicAdd(pixels[index+2], color.z);

	//Shape s = shapes[0];

	//uvec4 c = uvec4(shapeCount * 100, 0, 0, 0);

	//imageAtomicAdd(red, ivec2(gl_WorkGroupID.xy), c.x);
	//imageAtomicAdd(green, ivec2(gl_WorkGroupID.xy), c.y);
	//imageAtomicAdd(blue, ivec2(gl_WorkGroupID.xy), c.z);
}