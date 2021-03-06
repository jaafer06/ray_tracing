#version 430
#define size 10
#define inf (1./0.0)
#define pi 3.1415926535897932384626433832795

layout(local_size_x = size, local_size_y = size, local_size_z = 1) in;

uniform float time;
vec3 seed = vec3(time) + (vec3(gl_GlobalInvocationID.xyz) / vec3(gl_NumWorkGroups.xyz));
struct Triangle {
	vec3 position;
	vec3 a;
	float beta;
	vec3 m1;
	vec3 m2;
};

layout(std430, binding = 3) buffer triangleBuffer
{
	Triangle triangles[];
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

//float seed = (float(gl_LocalInvocationID.x) * size + float(gl_LocalInvocationID.y));
//float seed = gl_LocalInvocationIndex + gl_WorkGroupID.x * gl_WorkGroupSize.y + gl_WorkGroupID.x.y;
//float local_coord = (float(gl_LocalInvocationIndex * gl_NumWorkGroups.x * gl_NumWorkGroups.y)) / 100000000;
//float global_coord = (float(gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x)) / 100000000;
//float seed = 900*(local_coord + global_coord);

float random() {
	//seed = (asin(sin(seed * 500)) + pi/2) / pi;
	//return seed;
	seed += vec3(1.78 ,3.1, 50.78);
	return fract(sin(dot(seed, vec3(12.9898, 78.233, 51.112))) * 43758.5453123);
	//seed[0] = fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453123);
	//return seed[0];
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

vec3 randomOnHalfUnitShere(in vec3 normal) {

	float angle = random() * 2 * pi;
	float r = random();
	float distance = sqrt(1 - r * r);
	float x = cos(angle) * distance;
	float y = sin(angle) * distance;
	vec3 b1 = vec3(-normal[1], normal[0], 0);
	if (length(b1.xy) < 0.01) {
		b1 = normalize(vec3(0, -normal[2], normal[1]));
	} else {
		b1 = normalize(b1);
	}
	vec3 b2 = cross(normal, b1);
	vec3 result = b1 * x + b2 * y + normal * r;
	return result;
}

vec3 reflect(in vec3 incoming, in vec3 normal) {
	return normalize(incoming - 2 * dot(incoming, normal) * normal);
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
uniform uint triangleCount;

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
	float scale = sphere.transformation[0][0];
	vec3 position = sphere.transformation[3].xyz;

	vec3 oc = ray.origin - position;
	float b = dot(ray.direction, oc);
	float c = dot(oc, oc) - pow(scale, 2);
	float discriminant = b * b - c;
	if (discriminant < 0) {
		return inf;
	}
	else {
		return -b - sqrt(discriminant);
	}
};

vec3 sphereNormalAt(in Shape sphere, in vec3 p) {
	return normalize(p - sphere.transformation[3].xyz);
}

float hitBox(in Shape box, in Ray ray) {
	float scale = box.transformation[0][0];
	vec3 position = box.transformation[3].xyz;
	ray.origin = (1/scale) * (ray.origin - position);
	ray.direction = (1/scale) * ray.direction;

	vec3 sign = sign(ray.origin);
	vec3 t = (0.5 - ray.origin * sign) / (sign * ray.direction);
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
	float scale = box.transformation[0][0];
	vec3 position = box.transformation[3].xyz;
	p = (p - position) / scale;
	return normalize(step(vec3(0.499, 0.499, 0.499), abs(p)) * sign(p));
}

vec3 triangleNormalAt(in Triangle triangle, in vec3 rayDirection) {
	return triangle.a * sign(-dot(rayDirection, triangle.a));
}

float hit(in Shape shape, in Ray ray) {
	if (shape.type == 0) {
		return hitSphere(shape, ray);
	}
	else if (shape.type == 1) {
		return hitBox(shape, ray);
	}
	return -1;
}

float hitTriangle(in Triangle triangle, in Ray ray) {

	float k = (triangle.beta - dot(triangle.a, ray.origin)) / dot(triangle.a, ray.direction);
	if (k < 0) {
		return inf;
	}

	vec3 pointOnPlane = k * ray.direction + ray.origin - triangle.position;
	float x = dot(pointOnPlane, triangle.m1);
	float y = dot(pointOnPlane, triangle.m2);

	if (x > 0 && y > 0 && x + y < 1) {
		return k;
	}
	return inf;
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

void getClosestTriangleIndex(in Ray ray, out float distance, out int index) {
	distance = inf;
	index = -1;

	for (int i = 0; i < triangleCount; ++i) {
		float t = hitTriangle(triangles[i], ray);
		if (t < 1000 && t > 0.0001 && t < distance) {
			distance = t;
			index = i;
		}
	}
}

bool scatter(in Shape shape, in out Ray ray, float distance, in out vec3 color) {
	
	if (shape.material.type == 0) {
		color = shape.material.color * color;
		return false;
	} else if (shape.material.type == 1) {
		ray.origin = distance * ray.direction + ray.origin;
		ray.direction = randomOnHalfUnitShere(normalAt(shape, ray.origin));
		color = color * shape.material.color;
		return true;
	} else if (shape.material.type == 2) {
		ray.origin = distance * ray.direction + ray.origin;
		ray.direction = reflect(ray.direction, normalAt(shape, ray.origin));
		color = color * shape.material.color;
		return true;
	} else if (shape.material.type == 100) {
		ray.origin = distance * ray.direction + ray.origin;
		color = normalAt(shape, ray.origin);
		return false;
	}
	return false;
}

vec3 ray_color(inout Ray ray, uint maxDepth) {

	vec3 result = vec3(1, 1, 1);
	float distance;
	int index;
	float distance_triangle;
	int index_triangle;
	for (uint depth = 0; depth < maxDepth; ++depth) {
		getClosestShapeIndex(ray, distance, index);
		getClosestTriangleIndex(ray, distance_triangle, index_triangle);

		if (index == -1 && index_triangle == -1) {
			return vec3(0, 0, 0);
		}

		if (distance_triangle < distance) {
			vec3 normal = triangles[index_triangle].a * sign(-dot(ray.direction, triangles[index_triangle].a));
			ray.origin = distance_triangle * ray.direction + ray.origin;
			ray.direction = randomOnHalfUnitShere(normal);
			result = result * vec3(1, 0, 0);
			continue;
		}
		if (shapes[index].material.type == 0) {
			return result * shapes[index].material.color;
		}

		if (!scatter(shapes[index], ray, distance, result)) {
			return result;
		}
	}

	return vec3(0, 0, 0);
};


void main() {

	vec3 pixelCoordinate = upperLeft + worldStep * gl_WorkGroupID.x * right - worldStep * gl_WorkGroupID.y * up;
	float gridStep = worldStep / (size + 1);
	//vec3 rayOrigin = pixelCoordinate + gridStep * (gl_LocalInvocationID.x + 1) * right - gridStep * (gl_LocalInvocationID.y + 1) * up;
	vec3 rayOrigin = pixelCoordinate + worldStep * random() * right - worldStep * random() * up;
	vec3 rayDirection = normalize(rayOrigin - cameraPosition);
	Ray ray = Ray(rayOrigin, rayDirection);

	vec3 colorf = ray_color(ray, 10);
	uvec4 color = uvec4(255 * vec4(colorf, 1.));

	uint index = 4 * (gl_NumWorkGroups.x * gl_NumWorkGroups.y - ((gl_NumWorkGroups.x - gl_WorkGroupID.x) + gl_NumWorkGroups.x * gl_WorkGroupID.y));
	
	atomicAdd(pixels[index], color.x);
	atomicAdd(pixels[index + 1], color.y);
	atomicAdd(pixels[index + 2], color.z);
}