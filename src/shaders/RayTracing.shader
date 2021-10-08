#version 430
layout(local_size_x = 1, local_size_y = 1) in;
layout(std430, binding = 3) buffer sharedBuffer
{
	float data[];
};

void main() {
	// base pixel colour for image
	//vec4 pixel = vec4(1.0, 0.5, 0.0, 0.0);
	// get index in global work group i.e x,y position
	//ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

	//
	// interesting stuff happens here later
	//

	// output to a specific pixel in the image
	//imageStore(img_output, pixel_coords, pixel);
	uint index = gl_GlobalInvocationID.x * 4 + (gl_GlobalInvocationID.y * 4 * 500);
	data[index] += 0.01f;
}