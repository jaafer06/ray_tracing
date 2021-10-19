#version 430
#define size 10
#define hitTestConcurrency 10
#define inf (1./0.0)
#define pi 3.1415926535897932384626433832795

layout(local_size_x = size, local_size_y = size, local_size_z = hitTestConcurrency) in;

shared int[hitTestConcurrency] shapeIndex_;
shared float[hitTestConcurrency] distance_;
shared int sharedIndex;
//shared uint[size][size] colors_;

uniform float time;
vec2 seed = vec2(time) + (vec2(gl_GlobalInvocationID.xy) / vec2(gl_NumWorkGroups.xy));

layout(std430) buffer bb
{
	int[hitTestConcurrency] shapeIndexssbo_;
	float[hitTestConcurrency] distancessbo_;
	uint[size][size] colorsssbi_;
};

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

float random() {
	seed += 1;
	return fract(sin(dot(seed.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

vec3 randomOnUnitSphere() {
	float u1 = random();
	float u2 = random();
	float u3 = random();
	float u4 = random();
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

//bool Scatter(in Material material, in vec3 normal, in vec2 seed, out vec3 rayDirection) {
//	if (material.type == 0) {
//		return false;
//	} else if (material.type ==1) {
//		return lambertianScatter(material, normal, seed, rayDirection);
//	} else if (material.type == 2) {
//		return metalScatter(material, normal, seed, rayDirection);
//	}
//}
//
//bool lambertianScatter(in Material material, in vec3 normal, in vec2 seed, out vec3 rayDirection) {
//	rayDirection = normal + randomOnUnitSphere(seed + 10 * depth);
//	
//}


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

float hitBox(in Shape box, in Ray ray) {
	ray.origin = (box.transformation * vec4(ray.origin, 1)).xyz;
	ray.direction = (box.transformation * vec4(ray.direction, 1)- box.transformation[3]).xyz;

	vec3 sign = sign(ray.origin);
	vec3 t = (0.5 - ray.origin * sign)/ (sign * ray.direction);
	float minDistance = inf;
	for (uint index = 0; index < 3; ++index) {
		vec3 inPoint = ray.origin + ray.direction * t[index];
		vec3 inPoint_abs = abs(inPoint);
		float infNorm = max(inPoint_abs[0], max(inPoint_abs[1], inPoint_abs[2]));
		if (infNorm < 0.5001 && infNorm > 0.4999 && minDistance > t[index]) {
			minDistance = t[index];
		}
	}
	return minDistance;

};

vec3 boxNormalAt(in Shape box, in vec3 p) {
	p = (box.transformation * vec4(p, 1)).xyz;
	return normalize(step(vec3(0.499, 0.499, 0.499), abs(p)) * sign(p));
}

float hit(in Shape shape, in Ray ray) {
	if (shape.type == 0) {
		return hitSphere(shape, ray);
	} else if (shape.type == 1) {
		return hitBox(shape, ray);
	}
	return -1;
}

vec3 normalAt(in Shape shape, in vec3 p) {
	if (shape.type == 0) {
		return sphereNormalAt(shape, p);
	}
	else if (shape.type == 1) {
		return boxNormalAt(shape, p);
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

void computeShapesDistance(in Ray ray, in uint from, uint to) {
	float distance = inf;
	int index = -1;
	for (uint i = from; i < to; ++i) {
		float t = hit(shapes[i], ray);
		if (t < 1000 && t > 0.0001 && t < distance) {
			distance = t;
			index = int(i);
		}
	}
	//distancessbo_[gl_LocalInvocationID.z] = distance;
	sharedIndex = index;
	//atomicExchange(sharedIndex, index);
}

vec3 ray_color(inout Ray ray, uint maxDepth) {
	vec3 result = vec3(1, 1, 1);

	float distance;
	int index;
	for (uint depth = 0; depth < maxDepth; ++depth) {
		getClosestShapeIndex(ray, distance, index);
		if (index == -1) {
			//if (depth == 0) {
			//	float t = (upperLeft.y - ray.origin.y);
			//	return (1.0 - t)* vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
			//}
			//result = result / max(result.x, max(result.x, result.z));
			//return result;
			return vec3(0, 0, 0);
		}
		if (shapes[index].material.type == 0) {
			return result * shapes[index].material.color;
		}
		ray.origin = distance * ray.direction + ray.origin;
		ray.direction = normalize(normalAt(shapes[index], ray.origin) + randomOnUnitSphere());
		result = result  * shapes[index].material.color;
	}
	
	return vec3(0, 0, 0);
};



void main() {
	uint k = shapeCount / hitTestConcurrency;
	uint rest = shapeCount % hitTestConcurrency;
	uint load = k + uint(gl_LocalInvocationID.z<rest);
	uint startIndex = k * gl_LocalInvocationID.z + min(gl_LocalInvocationID.z, rest);

	vec3 pixelCoordinate = upperLeft + worldStep * gl_WorkGroupID.x * right - worldStep * gl_WorkGroupID.y * up;
	float gridStep = worldStep / (size+1);
	vec3 rayOrigin = pixelCoordinate + gridStep * (gl_LocalInvocationID.x+1) * right - gridStep * (gl_LocalInvocationID.y+1) * up;
	vec3 rayDirection = normalize(rayOrigin - cameraPosition);
	Ray ray = Ray(rayOrigin, rayDirection);

	for (uint depth = 0; depth < 10; ++depth) {
		computeShapesDistance(ray, startIndex, startIndex + load);
		//computeShapesDistance(ray, 0, shapeCount);
		//computeShapesDistance(ray, 0, 1);

	}

	/*vec3 colorf = ray_color(ray, 50);
	uvec4 color = uvec4(255 * vec4(colorf, 1.));*/

	//uint index = 4 * (gl_NumWorkGroups.x * gl_NumWorkGroups.y - ((gl_NumWorkGroups.x - gl_WorkGroupID.x) + gl_NumWorkGroups.x * gl_WorkGroupID.y));
	//atomicAdd(pixels[index], color.x);
	//atomicAdd(pixels[index+1], color.y);
	//atomicAdd(pixels[index+2], color.z);

	//atomicAdd(pixels[index], uint(shapes[0].transformation[0].x * 255));
	//atomicAdd(pixels[index + 1], uint(shapes[0].transformation[0].y * 255));
	//atomicAdd(pixels[index + 2], uint(shapes[0].transformation[0].z * 255));

}