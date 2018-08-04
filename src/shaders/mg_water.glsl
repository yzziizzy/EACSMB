#shader COMPUTE

#version 450

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;



//layout(rgba32f, binding = 0) uniform image2D sout;

layout(r32f) uniform writeonly restrict image2DArray sOut;
layout(rg8) uniform writeonly restrict image2DArray sVel;
uniform sampler2DArray sHeightMap;

uniform ivec2 axis;


uniform int waterIndex;

const int win = 1 + waterIndex;
const int wout = 2 - waterIndex;


void getInfo(ivec2 tc, ivec2 off, out float h, out float w) {
	float s = texelFetch(sHeightMap, ivec3(tc, 3) + ivec3(off.xy, 0), 0).r;
	h = s + texelFetch(sHeightMap, ivec3(tc, 0) + ivec3(off.xy, 0), 0).r;
	w = texelFetch(sHeightMap, ivec3(tc, win) + ivec3(off.xy, 0), 0).r;
//	h = imageLoad(sOut, ivec3(tc, 0) + ivec3(off.xy, 0)).r;
//	w = imageLoad(sOut, ivec3(tc, win) + ivec3(off.xy, 0)).r;
}


float maxDiff(float h, float w, float h2, float w2) {
	const float div = 4.0;
	float d;
	
	float t = h + w;
	float t2 = h2 + w2;
	
	d = (t2 - t) / 4.0;
	
// 	return clamp(d , -w / div, w2 / div);
	return clamp(d  , -w / div, w2 / div);
}




void main() {

	
	ivec2 tc = ivec2(gl_GlobalInvocationID.xy);
// 	vec2 sz = imageSize(sInput).xy;

	
	float o;
	float h, h1, h2, h3, h4;
	float w, w1, w2, w3, w4;
	
	getInfo(tc, ivec2(0, 0), h, w);
	getInfo(tc, ivec2(1,0), h1, w1);
	getInfo(tc, ivec2(0,1), h2, w2);
	getInfo(tc, ivec2(-1,0), h3, w3);
	getInfo(tc, ivec2(0,-1), h4, w4);
	
	
	float o1 = maxDiff(h, w, h1, w1);
	float o2 = maxDiff(h, w, h2, w2);
	float o3 = maxDiff(h, w, h3, w3); 
	float o4 = maxDiff(h, w, h4, w4);
// 	o = maxDiff(h, w, h1, w1) + maxDiff(h, w, h3, w3);
	
	o = o1 + o2 + o3 + o4;
	
	imageStore(sOut, ivec3(tc, wout), vec4(o + w));
	
	vec2 v = vec2(.25 * o1 + .25 * o3, .25 * o2 + .25 * o4);
	
	imageStore(sVel, ivec3(tc, 0), vec4(v * .5 + .5, 0,0));
	
	
	
}





