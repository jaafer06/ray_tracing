#version 430
#define totalSize 100
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(binding = 2, rgba32f) uniform image2D result;
layout(std430, binding = 2) buffer pixelBuffer
{
	uint pixels[];
};

uniform uint n_samples;

void main() {
	float n_samples_f = float(n_samples);
	uint index = 4 * gl_WorkGroupID.x + 4 * gl_NumWorkGroups.x * gl_WorkGroupID.y;
	vec4  p_color = imageLoad(result, ivec2(gl_WorkGroupID.xy));

	float red = float(pixels[index]) / (255. * totalSize);
	float green = float(pixels[index+1]) / (255. * totalSize);
	float blue = float(pixels[index+2]) / (255. * totalSize);
	vec4 new_color = vec4(red, green, blue, 1);
	if (n_samples_f != 1) {
		new_color = ((n_samples_f-1) / n_samples_f) * p_color + new_color * (1 / n_samples_f);
	}
	imageStore(result, ivec2(gl_WorkGroupID.xy), new_color);
	pixels[index] = 0;
	pixels[index+1] = 0;
	pixels[index+2] = 0;

}