#version 430
layout(local_size_x = 2, local_size_y = 2) in;
layout(std430, binding = 0) buffer sharedBuffer
{
	vec4[500][500] data;
};
shared mat4 foo;

void main() {
	uint index = gl_WorkGroupID.x * 4 + (gl_WorkGroupID.y * 4 * 500);

	foo[gl_LocalInvocationID.x][gl_LocalInvocationID.y] = 0.0005;
	memoryBarrierShared();

	if (gl_LocalInvocationID.x == 0 && gl_LocalInvocationID.y == 0) {
		vec4 t = foo * vec4(1.);
		data[gl_WorkGroupID.x][gl_WorkGroupID.y].x += t[0] + t[1] + t[2] + t[3];
	}
	//data[index] += 0.01f;
	//atomicAdd(data[index], 0.01f);
}