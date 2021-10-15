#version 430
#define size 15
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(binding = 5, r32ui) uniform uimage2D red;
layout(binding = 6, r32ui) uniform uimage2D green;
layout(binding = 7, r32ui) uniform uimage2D blue;

layout(binding = 2, rgba32f) uniform image2D result;

layout(std430, binding = 2) buffer pixelBuffer
{
	uint pixels[];
};

void main() {
	uint index = 4 * gl_WorkGroupID.x + 4 * gl_NumWorkGroups.x * gl_WorkGroupID.y;
	float red = float(pixels[index]) / (255. * size * size);
	float green = float(pixels[index+1]) / (255. * size * size);
	float blue = float(pixels[index+2]) / (255. * size * size);
	imageStore(result, ivec2(gl_WorkGroupID.xy), vec4(red, green, blue, 1));
}