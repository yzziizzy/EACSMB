

#shader VERTEX

#version 430 core

layout(std140) uniform; 

// per vertex
layout (location = 0) in vec2 pos_in;
layout (location = 1) in vec2 tex_in;

// per instance
layout (location = 2) in vec2 block_in;

uniform sampler2D sOffsetLookup;

out vec2 vs_tex;
out vec2 vs_tile;
out int vs_InstanceID;
out vec2 vs_rawTileOffset;
out vec2 vs_tileOffset;

void main() {
	vs_tex = tex_in;
	vs_tile = tex_in;
	vs_InstanceID = gl_InstanceID;

// 	vs_rawTileOffset = texelFetch(sOffsetLookup, ivec2(gl_InstanceID, 0), 0).rg; 
// 	vs_tileOffset = vs_rawTileOffset * 255* 255;
	gl_Position = vec4(pos_in.x + block_in.x, pos_in.y + block_in.x, 0, 1.0);
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

out vec2 te_tex[];
out vec2 te_tile[];;
out int te_InstanceID[];


bool inBox(vec4 v) {
	const float low = -1.0;
	const float high = 1.0;
	if(low > v.x || v.x > high) return false;
	if(low > v.y || v.y > high) return false;
	if(low > v.z || v.z > high) return false;
	
	return true;
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
		w0.z = texture(sHeightMap, vec3(vs_tile[0].xy, 0),0).r;
		w1.z = texture(sHeightMap, vec3(vs_tile[1].xy, 0),0).r;
		w2.z = texture(sHeightMap, vec3(vs_tile[2].xy, 0),0).r;
		w3.z = texture(sHeightMap, vec3(vs_tile[3].xy, 0),0).r;

		w0 = mvp * gl_in[0].gl_Position;
		w1 = mvp * gl_in[1].gl_Position;
		w2 = mvp * gl_in[2].gl_Position;
		w3 = mvp * gl_in[3].gl_Position;
		
		w0 /= w0.w;
		w1 /= w1.w;
		w2 /= w2.w;
		w3 /= w3.w;
		
		
		// cull patches outside the view frustum
		// TODO: sample height map and adjust corners
		if((!inBox(w0) && !inBox(w1) && !inBox(w2) && !inBox(w3))
			&& !insideQuad(w0.xz, w1.xz, w2.xz, w3.xz, vec2(1,1)) 
			&& !insideQuad(w0.xz, w1.xz, w2.xz, w3.xz, vec2(-1,1)) 
			&& !insideQuad(w0.xz, w1.xz, w2.xz, w3.xz, vec2(1,-1)) 
			&& !insideQuad(w0.xz, w1.xz, w2.xz, w3.xz, vec2(-1,-1)) 
		) {
			
			// discard the patch entirely
			gl_TessLevelOuter[0] = 0; 
			gl_TessLevelOuter[1] = 0; 
			gl_TessLevelOuter[2] = 0; 
			gl_TessLevelOuter[3] = 0;
			gl_TessLevelInner[0] = 0;
			gl_TessLevelInner[1] = 0;
			
			return;
		}

		
		float lod = 256;

		float f0 = clamp(distance(w1, w2) * lod, 1, 64);
		float f1 = clamp(distance(w0, w1) * lod, 1, 64);
		float f2 = clamp(distance(w3, w0) * lod, 1, 64);
		float f3 = clamp(distance(w2, w3) * lod, 1, 64);
	

		gl_TessLevelOuter[0] = f0; 
		gl_TessLevelOuter[1] = f1; 
		gl_TessLevelOuter[2] = f2; 
		gl_TessLevelOuter[3] = f3;
	
		gl_TessLevelInner[0] = mix(f1, f2, 0.5);
		gl_TessLevelInner[1] = mix(f2, f3, 0.5);
	}
	
	te_tile[gl_InvocationID] = vs_tile[gl_InvocationID];
	te_tex[gl_InvocationID] = vs_tex[gl_InvocationID];
	te_InstanceID[gl_InvocationID] = vs_InstanceID[gl_InvocationID];
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	
} 


#shader TESS_EVALUATION


#version 430 core

layout (quads, equal_spacing, ccw) in;

in vec2 te_tile[];
in vec2 te_tex[];
in int te_InstanceID[];


uniform sampler2DArray sHeightMap;


uniform mat4 mWorldView;
uniform mat4 mViewProj;




uniform vec2 winSize;


out vec3 t_tile;
flat out int ps_InstanceID;



void main(void){

	vec4 p1 = mix(gl_in[1].gl_Position, gl_in[0].gl_Position, gl_TessCoord.x);
	vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
	vec4 tmp = mix(p1, p2, gl_TessCoord.y);
	
	vec2 tp1 = mix(te_tex[1], te_tex[0], gl_TessCoord.x);
	vec2 tp2 = mix(te_tex[2], te_tex[3], gl_TessCoord.x);
	vec2 ttmp = mix(tp1, tp2, gl_TessCoord.y);
	
	vec2 tlp1 = mix(te_tile[1], te_tile[0], gl_TessCoord.x);
	vec2 tlp2 = mix(te_tile[2], te_tile[3], gl_TessCoord.x);
	vec2 tltmp = mix(tlp1, tlp2, gl_TessCoord.y);
	
	
	vec3 terrCoords = vec3(ttmp.xy, 0);
	
	float t = texture(sHeightMap, terrCoords, 0).r;
	
	tmp.z = t; //* .05; // .01 *  sin(gl_TessCoord.y*12) + .01 *sin(gl_TessCoord.x*12);

	gl_Position = (mViewProj * mWorldView) * tmp;
	t_tile = vec3(tltmp.xy, 1); 
	ps_InstanceID = te_InstanceID[0];
}


#shader FRAGMENT

 
#version 420




in vec3 t_tile;
flat in int ps_InstanceID;

layout(location = 0) out vec4 out_Selection;



void main(void) {
	ivec2 tile = ivec2(floor(t_tile.xy * 4));
	vec2 ftile = fract(t_tile.xy * 4);
	
	out_Selection = vec4(ftile.x, ftile.y, float(ps_InstanceID)/256.0, 1);
//	out_Selection = vec4(1 , 1,1 , 1);
//	out_Selection = vec4(10 , 10,10 , 10);
//	out_Selection = vec4(0.5,0.5,0.5,0.5);
}

