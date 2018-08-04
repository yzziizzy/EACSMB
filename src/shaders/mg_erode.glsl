#shader COMPUTE

#version 450

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;



//layout(rgba32f, binding = 0) uniform image2D sout;

layout(r32f) uniform writeonly restrict image2DArray iOut;
layout(rg8) uniform writeonly restrict image2DArray iVel;
uniform sampler2DArray sVel;
uniform sampler2DArray sHeightMap;





void main() {

	
	ivec2 tc = ivec2(gl_GlobalInvocationID.xy);
	
	vec2 v = texelFetch(sVel, ivec3(tc, 0), 0).rg;
	float o = length(v);
	
	
	float r = texelFetch(sHeightMap, ivec3(tc, 0), 0).r;
	// rock
	imageStore(iOut, ivec3(tc, 0), vec4(r - (.00051*o)));
	
	float s = texelFetch(sHeightMap, ivec3(tc, 3), 0).r;
	// soil
	imageStore(iOut, ivec3(tc, 3), vec4(s + (.00051*o)));
	
}





