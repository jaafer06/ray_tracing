#version 430
#define size 10
layout(local_size_x = size, local_size_y = size, local_size_z=1) in;

layout(binding = 0, rgba32f) uniform image2D imageBuffer;

shared uvec4 result;
shared vec4 resultf;


layout(std140, binding=1) uniform coordinates {
	vec3 cameraPosition;
	vec3 cameraDirection;
	vec3 upperLeft;
	vec3 up;
	vec3 right;
	float worldStep;
};


struct Ray {
	vec3 origin;
	vec3 direction;
};

bool hit(in Ray r) {
	vec3 oc = r.origin - vec3(0., 0., 0.);
	float b = dot(r.direction, oc);
	float c = dot(oc, oc) - 0.5 * 0.5;
	float discriminant = b * b - c;
	return discriminant > 0;
};

vec3 ray_color(in Ray ray, uint depth) {
	
	if (hit(ray)) {
		return vec3(1., 0., 0.);
	}

	//for (uint index = 0; index < depth; ++index) {

	//};
	float t = (upperLeft.y - ray.origin.y);

	return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);

};

void main() {
	if (gl_LocalInvocationID.xy == uvec2(0, 0)) {
		atomicExchange(result.x, 0);
		atomicExchange(result.y, 0);
		atomicExchange(result.z, 0);
		resultf = vec4(0.);
	}

	vec3 pixelCoordinate = upperLeft + worldStep * gl_WorkGroupID.x * right - worldStep * gl_WorkGroupID.y * up;
	float gridStep = worldStep / (size+1);
	vec3 rayOrigin = pixelCoordinate + gridStep * (gl_LocalInvocationID.x+1) * right - gridStep * (gl_LocalInvocationID.y+1) * up;
	vec3 rayDirection = normalize(rayOrigin - cameraPosition);
	Ray ray = Ray(rayOrigin, rayDirection);
	uvec4 color = uvec4(255 * vec4(ray_color(ray, 0), 1.));

	//resultf += (1./float(size*size))* vec4(ray_color(ray, 0), 1.);
	//memoryBarrierShared();

	atomicAdd(result.x, color.x);
	//atomicAdd(result.y, color.y);
	//atomicAdd(result.z, color.z);
	//memoryBarrierShared();
	//barrier();
	//if (gl_LocalInvocationID.xy == uvec2(1, 0)) {
	//	//data[gl_WorkGroupID.x][gl_WorkGroupID.y].x += t[0] + t[1] + t[2] + t[3];
	//	data[gl_WorkGroupID.y][gl_WorkGroupID.x] = vec4(result) /(255*size*size);
	//	//data[gl_WorkGroupID.x][gl_WorkGroupID.y] = vec4(ray_color(ray, 0), 1.);
	//	//data[gl_WorkGroupID.y][gl_WorkGroupID.x] = resultf;
	//}

	//imageAtomicAdd(imageBuffer, ivec2(gl_WorkGroupID.xy), vec4(1., 0., 0., 1.));
	imageStore(imageBuffer, ivec2(gl_WorkGroupID.xy), vec4(1, 1, 0, 1));


}