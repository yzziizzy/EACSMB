#shader COMPUTE

#version 450

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;



//layout(rgba32f, binding = 0) uniform image2D sout;

layout(r32f) uniform image2DArray sOut;
uniform sampler2DArray sHeightMap;
uniform int waterIndex;

const int win = 1 + waterIndex;
const int wout = 2 - waterIndex;


void getInfo(ivec2 tc, ivec2 off, out float h, out float w) {
	h = texelFetch(sHeightMap, ivec3(tc, 0) + ivec3(off.xy, 0), 0).r;
	w = texelFetch(sHeightMap, ivec3(tc, win) + ivec3(off.xy, 0), 0).r;
}


float maxDiff(float h, float w, float h_o, float w_o) {
	const float div = 4;
	
	float t = h + w;
	float t_o = h_o + w_o;
	
	float d = (t_o - t) / 2.0;
	
	return clamp(d, -w/div, w_o/div);
}




void main() {

	
	ivec2 tc = ivec2(gl_GlobalInvocationID.xy);
// 	vec2 sz = imageSize(sInput).xy;
	
// 	float h_xp = texelFetchOffset(sHeightMap, ivec3(tc, 0), 0, ivec2(1, 0)).r;
// 	float h    = texelFetchOffset(sHeightMap, ivec3(tc, 0), 0, ivec2(0, 0)).r;
// 	float h_xm = texelFetchOffset(sHeightMap, ivec3(tc, 0), 0, ivec2(-1, 0)).r;
// 	
// 	float w_xp = texelFetchOffset(sHeightMap, ivec3(tc, win), 0, ivec2(1, 0)).r;
// 	float w    = texelFetchOffset(sHeightMap, ivec3(tc, win), 0, ivec2(0, 0)).r;
// 	float w_xm = texelFetchOffset(sHeightMap, ivec3(tc, win), 0, ivec2(-1, 0)).r;
// 	
// 	float h_xp = texelFetch(sHeightMap, ivec3(tc, 0) + ivec3(1, 0, 0), 0).r;
// 	float h    = texelFetch(sHeightMap, ivec3(tc, 0) + ivec3(0, 0, 0), 0).r;
// 	float h_xm = texelFetch(sHeightMap, ivec3(tc, 0) + ivec3(-1, 0, 0), 0).r;
// 	
// 	float w_xp = texelFetch(sHeightMap, ivec3(tc, win) + ivec3(1, 0, 0), 0).r;
// 	float w    = texelFetch(sHeightMap, ivec3(tc, win) + ivec3(0, 0, 0), 0).r;
// 	float w_xm = texelFetch(sHeightMap, ivec3(tc, win) + ivec3(-1, 0, 0), 0).r;
	
	float h, h_xp, h_xm, h_yp, h_ym;
	float w, w_xp, w_xm, w_yp, w_ym;
	
	getInfo(tc, ivec2(0, 0), h, w);
	getInfo(tc, ivec2(1,0), h_xp, w_xp);
	getInfo(tc, ivec2(0,1), h_yp, w_yp);
	getInfo(tc, ivec2(-1,0), h_xp, w_xp);
	getInfo(tc, ivec2(0,-1), h_ym, w_ym);
	
	
	float o = maxDiff(h, w, h_xp, w_xp) + maxDiff(h, w, h_xm, w_xm) + maxDiff(h, w, h_yp, w_yp) + maxDiff(h, w, h_ym, w_ym);
	
// 	o = clamp(w + o , 0, w + w_xp + w_yp + w_xm + w_ym);
	
	o = w + o;
	
	imageStore(sOut, ivec3(tc, wout), vec4(o));
	
}





