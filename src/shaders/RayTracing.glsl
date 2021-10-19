#version 430
#define size 20
#define inf (1./0.0)
#define pi 3.1415926535897932384626433832795
#define stepSize 10e-3
#extension GL_ARB_gpu_shader_int64 : enable

layout(local_size_x = size, local_size_y = size, local_size_z=1) in;

uniform float time;
vec2 seed = vec2(time) + (vec2(gl_GlobalInvocationID.xy) / vec2(gl_NumWorkGroups.xy));

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

struct Shape {
	uint64_t type_pos;
};

uint get_type(in Shape shape) {
	return uint(shape.type_pos & 0xf);
}

vec3 get_position(in Shape shape) {
	float x = float(shape.type_pos << 40 >> 44) * stepSize;
	float y = float(shape.type_pos << 20 >> 44) * stepSize;
	float z = float(shape.type_pos >> 44) * stepSize;
	return vec3(x, y, z);
}

layout(std430, binding = 3) buffer shapes2Buffer
{
	Shape shapes[5];
};

struct Shape2 {
	uint type;
	vec3 pos;
};

shared Shape2[5] myshapes;

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

uniform uint shapeCount;

struct Ray {
	vec3 origin;
	vec3 direction;
};


float hitSphere(in Shape2 sphere, in Ray ray) {
	vec3 oc = ray.origin - sphere.pos;
	float b = dot(ray.direction, oc);
	float c = dot(oc, oc) - pow(0.5, 2);
	float discriminant = b * b - c;
	if (discriminant < 0) {
		return inf;
	} else {
		return -b - sqrt(discriminant);
	}
};

vec3 sphereNormalAt(in Shape2 sphere, in vec3 p) {
	return normalize(p - sphere.pos);
}

float hitBox(in Shape2 box, in Ray ray) {
	ray.origin = ray.origin - box.pos;
	//ray.direction = (box.transformation * vec4(ray.direction, 1)- box.transformation[3]).xyz;

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

vec3 boxNormalAt(in Shape2 box, in vec3 p) {
	p = p - box.pos;
	return normalize(step(vec3(0.499, 0.499, 0.499), abs(p)) * sign(p));
}

float hit(in Shape2 shape, in Ray ray) {
	if (shape.type == 0) {
		return hitSphere(shape, ray);
	} else if (shape.type == 1) {
		return hitBox(shape, ray);
	}
	return -1;
}

vec3 normalAt(in Shape2 shape, in vec3 p) {
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
		float t = hit(myshapes[i], ray);
		if (t < 1000 && t > 0.00001 && t < distance) {
			distance = t;
			index = i;
		}
	}
}

vec3 ray_color(inout Ray ray, uint maxDepth) {
	vec3 result = vec3(1, 1, 1);

	float distance;
	int index;
	for (uint depth = 0; depth < maxDepth; ++depth) {
		getClosestShapeIndex(ray, distance, index);
		if (index == -1) {
			if (depth == 0) {
				float t = (upperLeft.y - ray.origin.y);
				return ((1.0 - t)* vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0));
			}
			//result = result / max(result.x, max(result.x, result.z));
			//return result;
			float t = (upperLeft.y - ray.origin.y);
			return result * ((1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0)) * 0.5;
			//return vec3(0, 0, 0);
		}

		//if (shapes[index].material.type == 0) {
		//	return result * shapes[index].material.color;
		//}
		ray.origin = distance * ray.direction + ray.origin;
		ray.direction = normalize(normalAt(myshapes[index], ray.origin) + randomOnUnitSphere());
		result = result  * vec3(1, 0, 0);
	}
	
	return vec3(0, 0, 0);
};


void main() {
	if (gl_LocalInvocationID.x <= 4 && gl_LocalInvocationID.y == 0 && gl_LocalInvocationID.z == 0) {
		Shape2 s;
		s.type = get_type(shapes[gl_LocalInvocationID.x]);
		s.pos = get_position(shapes[gl_LocalInvocationID.x]);
		myshapes[gl_LocalInvocationID.x] = s;
	}
	barrier();
	vec3 pixelCoordinate = upperLeft + worldStep * gl_WorkGroupID.x * right - worldStep * gl_WorkGroupID.y * up;
	float gridStep = worldStep / (size+1);
	vec3 rayOrigin = pixelCoordinate + gridStep * (gl_LocalInvocationID.x+1) * right - gridStep * (gl_LocalInvocationID.y+1) * up;
	vec3 rayDirection = normalize(rayOrigin - cameraPosition);
	Ray ray = Ray(rayOrigin, rayDirection);

	vec3 colorf = ray_color(ray, 50);
	uvec4 color = uvec4(255 * vec4(colorf, 1.));

	//uint type = uint(shapes[0].type_pos & 0xf);
	//float x = float(shapes[0].type_pos << 40 >> 44 ) * stepSize;
	//float y = float(shapes[0].type_pos << 20 >> 44) * stepSize;
	//float z = float(shapes[0].type_pos >> 44) * stepSize;
	//uvec3 color = uvec3(x, 0, 0);
	uint index = 4 * (gl_NumWorkGroups.x * gl_NumWorkGroups.y - ((gl_NumWorkGroups.x - gl_WorkGroupID.x) + gl_NumWorkGroups.x * gl_WorkGroupID.y));
	atomicAdd(pixels[index], color.x);
	atomicAdd(pixels[index+1], color.y);
	atomicAdd(pixels[index+2], color.z);

}