 

#shader VERTEX

#version 430 core

layout(std140) uniform; 

layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec2 tex_in;
layout (location = 2) in vec2 tile_in;

uniform sampler2D sOffsetLookup;

out vec2 vs_tex;
out vec2 vs_tile;
out int vs_InstanceID;
out vec2 vs_rawTileOffset;
out vec2 vs_tileOffset;

void main() {
	vs_tex = tex_in;
	vs_tile = tile_in;
	vs_InstanceID = gl_InstanceID;
	vs_rawTileOffset = texelFetch(sOffsetLookup, ivec2(gl_InstanceID, 0), 0).rg; 
	vs_tileOffset = vs_rawTileOffset * 255* 255;
	
	
	gl_Position = vec4(pos_in.x + vs_tileOffset.r, pos_in.y + vs_tileOffset.g, pos_in.z, 1.0);
}



#shader TESS_CONTROL

#version 430 core

layout(std140) uniform; 

layout (vertices = 4) out;

uniform sampler2DArray sHeightMap;



uniform mat4 mView;
uniform mat4 mProj;



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
		mat4 mvp = mProj * mView;

		vec4 w0, w1, w2, w3;
		
		// this makes culling better, but still not completely correct
		w0.z = texture(sHeightMap, vec3(vs_tile[0].xy, vs_InstanceID[0]),0).r;
		w1.z = texture(sHeightMap, vec3(vs_tile[1].xy, vs_InstanceID[1]),0).r;
		w2.z = texture(sHeightMap, vec3(vs_tile[2].xy, vs_InstanceID[2]),0).r;
		w3.z = texture(sHeightMap, vec3(vs_tile[3].xy, vs_InstanceID[3]),0).r;

		w0 = mvp * gl_in[0].gl_Position;
		w1 = mvp * gl_in[1].gl_Position;
		w2 = mvp * gl_in[2].gl_Position;
		w3 = mvp * gl_in[3].gl_Position;
		
		w0 /= w0.w;
		w1 /= w1.w;
		w2 /= w2.w;
		w3 /= w3.w;
		
		// cull patches outside the view frustum
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

		float lod = 128;
		
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


uniform mat4 mView;
uniform mat4 mProj;


uniform vec2 winSize;

const vec2 size = vec2(2,0.0);
const ivec3 off = ivec3(-1,0,1);



out vec2 texCoord;
out vec2 t_tile;
out vec4 te_normal;
flat out int ps_InstanceID;
// flat out vec2 ps_rawTileOffset;
// flat out vec2 ps_tileOffset;

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
	
	vec3 terrCoords = vec3(ttmp.xy, te_InstanceID[0]);
	
	float t = texture(sHeightMap, terrCoords, 0).r;
	
	// normals. remember that z is still up at this point
	float xm1 = textureOffset(sHeightMap, terrCoords, off.xy).x;
	float xp1 = textureOffset(sHeightMap, terrCoords, off.zy).x;
	float ym1 = textureOffset(sHeightMap, terrCoords, off.yx).x;
	float yp1 = textureOffset(sHeightMap, terrCoords, off.yz).x;

	float sx = (xp1 - xm1);
	float sy = (yp1 - ym1);

	te_normal = vec4(normalize(vec3(sx, sy, 1.0)), 1);
	//te_normal = vec4(normalize(vec3(0,0,1)), 1.0);

	tmp.z = t; // .01 *  sin(gl_TessCoord.y*12) + .01 *sin(gl_TessCoord.x*12);
	
	gl_Position = (mProj * mView) * tmp;
	t_tile =  tltmp;
	texCoord = ttmp;
	ps_InstanceID = te_InstanceID[0];
}


#shader FRAGMENT

 
#version 420



uniform vec3 cursorPos;

// uniform float cursorRad;


in vec2 texCoord;
in vec2 t_tile;
in vec4 te_normal;
flat in int ps_InstanceID;

const vec2 eye = vec2(500,500);

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;
//layout(location = 2) out vec4 out_Selection;

#define UNIT 1
#define HALFUNIT .5

uniform sampler2D sBaseTex;
uniform sampler2DArray sDiffuse;
uniform isampler2DArray sMap; // 0 = zones, 1 = surfaceTex
uniform sampler1D sZoneColors;
uniform sampler2D sOffsetLookup;


void main(void) {
	
	ivec2 tile = ivec2(floor(texCoord * 256));
	
	int zoneIndex = texelFetch(sMap, ivec3(tile.xy,0), 0).r;
	vec4 zoneColor = texelFetch(sZoneColors, zoneIndex, 0);
	
	float scaleNear = 4;
	float scaleFar = 16;
	
	float q = mod(texCoord.x * 256, scaleFar) / scaleFar;
	float r = mod(texCoord.y * 256, scaleFar) / scaleFar;
	
	float qn = mod(texCoord.x * 256, scaleNear) / scaleNear;
	float rn = mod(texCoord.y * 256, scaleNear) / scaleNear;
	
	
	vec2 thisPixelOffset = texelFetch(sOffsetLookup, ivec2(ps_InstanceID, 0), 0).rg;
	vec2 rawTileOffset = texelFetch(sOffsetLookup, ivec2(cursorPos.z, 0), 0).rg; 
	
	vec2 realtile = (rawTileOffset * 256.0 * 256.0) + cursorPos.xy;
	vec2 realpix = (thisPixelOffset * 256.0 * 256.0) + t_tile; 
	
	float d = distance(realpix, realtile);
// 	float d = distance(t_tile, cursorPos.xy);
	if(d < 100) {
		float s = clamp((d - 95) / 10, 0.0, 1.0);
		q = mix(qn , q, s);
		r = mix(rn , r, s);
	}
	
	
	float ei1 = smoothstep(0.0, 0.02, q);
	float ei2 = 1.0 - smoothstep(0.98, 1.0, q);
	float ei3 = smoothstep(0.0, 0.02, r);
	float ei4 = 1.0 - smoothstep(0.98, 1.0, r);
	
	//out_Color = vec4(t_tile.x, t_tile.y,1 ,1.0);
	
 	
//  	vec4 tc = texture2D(sBaseTex, texCoord);
 	vec4 tc = texture(sDiffuse, vec3(texCoord.xy, t_tile.x  ));
//  	vec4 tc = vec4(texture(sMap, vec3(texCoord, 1)).rgb * 128, 1.0);
 	
 	// "in cursor"
 	bool incx = t_tile.x > cursorPos.x && t_tile.x < cursorPos.x + UNIT;
 	bool incy = t_tile.y > cursorPos.y && t_tile.y < cursorPos.y + UNIT;
 	bool inct = ps_InstanceID == floor(cursorPos.z);
 	
	//float distToCursor = length(gl_TessCoord.xy - cursorPos);
	vec4 cursorIntensity = (incx && incy && inct) ? vec4(0,10.0,10.0, 1.0) : vec4(1,1,1,1) ;//0 cursorRad - exp2(-1.0*distToCursor*distToCursor);
	
	d /= (255);
	
	vec4 lineFactor = vec4(min(min(ei1, ei2), min(ei3, ei4)), 0,0,1).rrra;
	vec4 lineColor = vec4(0,0,0,1.0);
	vec4 tc2 = mix(lineColor, tc, lineFactor.r);
	
//	out_Selection = vec4(floor(t_tile.x) / 256, floor(t_tile.y) / 256, ps_InstanceID, 1);
//	out_Normal = vec4(normalize(vec3((te_normal.x + 1) / 2, (te_normal.z + 1) / 2, (te_normal.y + 1) / 2)), 1);
	out_Normal = vec4(te_normal.xyz * .5 + .5, 1.0);
//	out_Normal = vec4(normalize(vec3(0,1,0)),1);
	
	out_Color =  (zoneColor * .2 + tc2) * cursorIntensity;// * lineFactor; //(1.0, 0, .5, .6);
 	//out_Color = vec4(texCoord.xy , 1, 1);
	
}

