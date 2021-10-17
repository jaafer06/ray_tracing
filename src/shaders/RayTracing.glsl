#version 430
#define size 20
#define inf (1./0.0)
#define pi 3.1415926535897932384626433832795

layout(local_size_x = size, local_size_y = size, local_size_z=1) in;

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

float random(in vec2 st) {
	return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

vec3 randomOnUnitSphere(in vec2 st) {
	float u1 = random(st);
	float u2 = random(st+1);
	float u3 = random(st+2);
	float u4 = random(st+3);
	float n1 = sqrt(-2 * log(u1)) * cos(2 * pi * u2);
	float n2 = sqrt(-2 * log(u1)) * sin(2 * pi * u2);
	float n3 = sqrt(-2 * log(u3)) * cos(2 * pi * u4);
	return normalize(vec3(n1, n2, n3));
}

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

uniform float time;
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

float hitSphere(in Shape sphere, in Ray ray) {
	vec3 oc = ray.origin - sphere.transformation[3].xyz;
	float b = dot(ray.direction, oc);
	float c = dot(oc, oc) - pow(sphere.transformation[0][0], 2);
	float discriminant = b * b - c;
	if (discriminant < 0) {
		return inf;
	} else {
		return -b - sqrt(discriminant);
	}
};

vec3 sphereNormalAt(in Shape sphere, in vec3 p) {
	return normalize(p - sphere.transformation[3].xyz);
}

float hit(in Shape shape, in Ray ray) {
	if (shape.type == 0) {
		return hitSphere(shape, ray);
	} else if (shape.type == 1) {
		return -1;
	}
	return -1;
}

vec3 normalAt(in Shape shape, in vec3 p) {
	if (shape.type == 0) {
		return sphereNormalAt(shape, p);
	}
	else if (shape.type == 1) {
		return vec3(1);
	}
	return vec3(1);
}

void getClosestShapeIndex(in Ray ray, out float distance, out int index) {
	distance = inf;
	index = -1;
	for (int i = 0; i < shapeCount; ++i) {
		float t = hit(shapes[i], ray);
		if (t < 1000 && t > 0.0001 && t < distance) {
			distance = t;
			index = i;
		}
	}
}

vec3 ray_color(inout Ray ray, uint maxDepth, in vec2 seed) {
	vec3 result = vec3(1, 1, 1);

	float distance;
	int index;
	for (uint depth = 0; depth < maxDepth; ++depth) {
		getClosestShapeIndex(ray, distance, index);
		if (index == -1) {
			if (depth == 0) {
				float t = (upperLeft.y - ray.origin.y);
				return (1.0 - t)* vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
			}
			//result = result / max(result.x, max(result.x, result.z));
			return result;
		}
		ray.origin = distance * ray.direction + ray.origin;
		ray.direction = normalize(normalAt(shapes[index], ray.origin) + randomOnUnitSphere(seed + 10*depth));
		result = result * pow(0.8, depth+1) * shapes[index].material.color;

	}
	
	return vec3(0, 0, 0);
};

/*vec3 normal = normalAt(shapes[index], ray.origin);
		ray.origin = t * ray.direction + ray.origin;
		ray.direction = normal + (randomOnUnitSphere(seed));*/

void main() {
	vec2 seed = vec2(time) + gl_GlobalInvocationID.xy;
	vec3 pixelCoordinate = upperLeft + worldStep * gl_WorkGroupID.x * right - worldStep * gl_WorkGroupID.y * up;
	float gridStep = worldStep / (size+1);
	vec3 rayOrigin = pixelCoordinate + gridStep * (gl_LocalInvocationID.x+1) * right - gridStep * (gl_LocalInvocationID.y+1) * up;
	vec3 rayDirection = normalize(rayOrigin - cameraPosition);
	Ray ray = Ray(rayOrigin, rayDirection);

	vec3 colorf = ray_color(ray, 50, seed);
	uvec4 color = uvec4(255 * vec4(colorf, 1.));

	uint index = 4 * (gl_NumWorkGroups.x * gl_NumWorkGroups.y - (gl_WorkGroupID.x + gl_NumWorkGroups.x * gl_WorkGroupID.y));
	atomicAdd(pixels[index], color.x);
	atomicAdd(pixels[index+1], color.y);
	atomicAdd(pixels[index+2], color.z);

	//atomicAdd(pixels[index], uint(shapes[0].transformation[0].x * 255));
	//atomicAdd(pixels[index + 1], uint(shapes[0].transformation[0].y * 255));
	//atomicAdd(pixels[index + 2], uint(shapes[0].transformation[0].z * 255));

}