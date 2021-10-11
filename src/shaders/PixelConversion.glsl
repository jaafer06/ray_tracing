#version 430
#define size 15
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(binding = 5, r32ui) uniform uimage2D red;
layout(binding = 6, r32ui) uniform uimage2D green;
layout(binding = 7, r32ui) uniform uimage2D blue;

layout(binding = 2, rgba32f) uniform image2D result;


void main() {
	float red = float(imageLoad(red, ivec2(gl_WorkGroupID.xy))) / (255. * size * size);
	float green = float(imageLoad(green, ivec2(gl_WorkGroupID.xy))) / (255.* size * size);
	float blue = float(imageLoad(blue, ivec2(gl_WorkGroupID.xy))) / (255.* size * size);
	imageStore(result, ivec2(gl_WorkGroupID.xy), vec4(red, green, blue, 1));
}