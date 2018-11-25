 

#shader VERTEX

#version 430 core

layout(std140) uniform; 

// per vertex
layout (location = 0) in vec2 pos_in;
layout (location = 1) in vec2 tex_in;

// per instance
layout (location = 2) in vec2 block_in;


// uniform sampler2D sOffsetLookup;

out vec2 vs_tex;
out vec2 vs_tile;
out int vs_InstanceID;
out vec2 vs_rawTileOffset;
out vec2 vs_tileOffset;

void main() {
	vs_tex = (tex_in / 2) + ((block_in) * .5);
	vs_tile = (tex_in / 2) + ((block_in) * .5); //vec2(0,0);//tile_in;
	vs_InstanceID = gl_InstanceID;
	
	gl_Position = vec4(pos_in.x + block_in.x * 256.0, pos_in.y + block_in.y * 256.0, 0, 1.0);
}



#shader TESS_CONTROL

#version 430 core

layout(std140) uniform; 

layout (vertices = 4) out;

uniform sampler2DArray sHeightMap;



uniform mat4 mWorldView;
uniform mat4 mViewProj;



in vec2 vs_tex[];
in vec2 vs_tile[];
in int vs_InstanceID[];
// in vec2 vs_rawTileOffset[];
// in vec2 vs_tileOffset[];

out vec2 te_tex[];
out vec2 te_tile[];
out int te_InstanceID[];
// out vec2 te_rawTileOffset[];
// out vec2 te_tileOffset[];


bool inBox(vec4 v) {
	const float low = -1.0;
	const float high = 1.0;
	if(low > v.x || v.x > high) return false;
	if(low > v.y || v.y > high) return false;
	if(low > v.z || v.z > high) return false;
	
	return true;
}

vec4 leftPlane(mat4 m) {
	return vec4(
		m[0][3] + m[0][0],
		m[1][3] + m[1][0],
		m[2][3] + m[2][0],
		m[3][3] + m[3][0]
	);
}
vec4 rightPlane(mat4 m) {
	return vec4(
		m[0][3] - m[0][0],
		m[1][3] - m[1][0],
		m[2][3] - m[2][0],
		m[3][3] - m[3][0]
	);
}
vec4 bottomPlane(mat4 m) {
	return vec4(
		m[0][3] + m[0][1],
		m[1][3] + m[1][1],
		m[2][3] + m[2][1],
		m[3][3] + m[3][1]
	);
}
vec4 topPlane(mat4 m) {
	return vec4(
		m[0][3] - m[0][1],
		m[1][3] - m[1][1],
		m[2][3] - m[2][1],
		m[3][3] - m[3][1]
	);
}
vec4 nearPlane(mat4 m) {
	return vec4(
		m[0][3] + m[0][2],
		m[1][3] + m[1][2],
		m[2][3] + m[2][2],
		m[3][3] + m[3][2]
	);
}
vec4 farPlane(mat4 m) {
	return vec4(
		m[0][3] - m[0][2],
		m[1][3] - m[1][2],
		m[2][3] - m[2][2],
		m[3][3] - m[3][2]
	);
}

bool linePlaneParallel(vec4 plane, vec4 line) {
	return abs(dot(plane.xyz, line.xyz)) < 0.001;
}

bool leftOf(vec2 l1, vec2 l2, vec2 p) { 
	return (l2.x - l1.x) * (p.y - l1.y) > (l2.y - l1.y) * (p.x - l1.x);
}

bool insideTriangle(vec2 t1, vec2 t2, vec2 t3, vec2 p) {
	bool s1 = leftOf(t1, t2, p);
	bool s2 = leftOf(t2, t3, p);
	if(s1 != s2) return false;
	
	bool s3 = leftOf(t3, t1, p);
	return s2 == s3; 
}

bool insideQuad(vec2 t1, vec2 t2, vec2 t3, vec2 t4, vec2 p) {
	return insideTriangle(t1, t2, t3, p) && insideTriangle(t3, t4, t1, p);
}


void main() {

	if(gl_InvocationID == 0) {
		mat4 mvp = mViewProj * mWorldView;

		vec4 w0, w1, w2, w3;
		
		// this makes culling better, but still not completely correct
		w0 = gl_in[0].gl_Position;
		w1 = gl_in[1].gl_Position;
		w2 = gl_in[2].gl_Position;
		w3 = gl_in[3].gl_Position;
		
		w0.z = texture(sHeightMap, vec3(vs_tile[0].xy, 0),0).r;
		w1.z = texture(sHeightMap, vec3(vs_tile[1].xy, 0),0).r;
		w2.z = texture(sHeightMap, vec3(vs_tile[2].xy, 0),0).r;
		w3.z = texture(sHeightMap, vec3(vs_tile[3].xy, 0),0).r;

		w0 = mvp * w0;
		w1 = mvp * w1; 
		w2 = mvp * w2; 
		w3 = mvp * w3;
		
		w0 /= w0.w;
		w1 /= w1.w;
		w2 /= w2.w;
		w3 /= w3.w;
		
		// cull patches outside the view frustum
		bool box = (!inBox(w0) && !inBox(w1) && !inBox(w2) && !inBox(w3));
		bool quad = 
			   !insideQuad(w0.xy, w1.xy, w2.xy, w3.xy, vec2(1.01,1.01)) 
			&& !insideQuad(w0.xy, w1.xy, w2.xy, w3.xy, vec2(-1.01,1.01)) 
			&& !insideQuad(w0.xy, w1.xy, w2.xy, w3.xy, vec2(1.01,-1.01)) 
			&& !insideQuad(w0.xy, w1.xy, w2.xy, w3.xy, vec2(-1.01,-1.01)); 
		
		if(quad && box) {
			
			// discard the patch entirely
			gl_TessLevelOuter[0] = 0; 
			gl_TessLevelOuter[1] = 0; 
			gl_TessLevelOuter[2] = 0; 
			gl_TessLevelOuter[3] = 0;
			gl_TessLevelInner[0] = 0;
			gl_TessLevelInner[1] = 0;
			
			return;
		}

		float lod = 256; // lower means worse quality. 128 is optimal
		
		float f0 = clamp(distance(w1, w2) * lod, 1, 64);
		float f1 = clamp(distance(w0, w1) * lod, 1, 64);
		float f2 = clamp(distance(w3, w0) * lod, 1, 64);
		float f3 = clamp(distance(w2, w3) * lod, 1, 64);
	

// 		gl_TessLevelOuter[0] = 4; 
// 		gl_TessLevelOuter[1] = 4; 
// 		gl_TessLevelOuter[2] = 4; 
// 		gl_TessLevelOuter[3] = 4;
// 	
// 		gl_TessLevelInner[0] = 4;
// 		gl_TessLevelInner[1] = 4;

		
		gl_TessLevelOuter[0] = f0; 
		gl_TessLevelOuter[1] = f1; 
		gl_TessLevelOuter[2] = f2; 
		gl_TessLevelOuter[3] = f3;
	
		gl_TessLevelInner[0] = mix(f1, f2, 0.5);
		gl_TessLevelInner[1] = mix(f2, f3, 0.5);
		
	}
		
// 	te_rawTileOffset[gl_InvocationID] = vs_rawTileOffset[gl_InvocationID];
// 	te_tileOffset[gl_InvocationID] = vs_rawTileOffset[gl_InvocationID];
	
	te_tex[gl_InvocationID] = vs_tex[gl_InvocationID];
	te_tile[gl_InvocationID] = vs_tile[gl_InvocationID];
	te_InstanceID[gl_InvocationID] = vs_InstanceID[gl_InvocationID];
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	
}


#shader TESS_EVALUATION


#version 430 core

layout (quads, equal_spacing, ccw) in;

in vec2 te_tex[];
in vec2 te_tile[];
in int te_InstanceID[];
// in vec2 te_rawTileOffset[];
// in vec2 te_tileOffset[];

uniform sampler2DArray sHeightMap;


uniform mat4 mWorldView;
uniform mat4 mViewProj;


void main(void){

	vec4 p1 = mix(gl_in[1].gl_Position, gl_in[0].gl_Position, gl_TessCoord.x);
	vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
	vec4 tmp = mix(p1, p2, gl_TessCoord.y);
	
	vec2 tp1 = mix(te_tex[1], te_tex[0], gl_TessCoord.x);
	vec2 tp2 = mix(te_tex[2], te_tex[3], gl_TessCoord.x);
	vec2 ttmp = mix(tp1, tp2, gl_TessCoord.y);
	
	vec3 terrCoords = vec3(ttmp.xy, 0);
	
	float t = texture(sHeightMap, terrCoords, 0).r;
	
	tmp.z = t; // .01 *  sin(gl_TessCoord.y*12) + .01 *sin(gl_TessCoord.x*12);
	
	gl_Position = (mViewProj * mWorldView) * tmp;
}


#shader FRAGMENT

 
#version 420


void main(void) {
	// just write to depth buffer
}

