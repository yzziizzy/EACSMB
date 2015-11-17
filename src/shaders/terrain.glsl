

#shader VERTEX

#version 430 core

layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec2 tex_in;
layout (location = 2) in vec2 tile_in;


out vec2 vs_tex;
out vec2 vs_tile;

void main() {
	vs_tex = tex_in;
	vs_tile = tile_in;
	gl_Position = vec4(pos_in.xyz, 1.0);
}



#shader TESS_CONTROL

#version 430 core


layout (vertices = 4) out;


in vec2 vs_tex[];
in vec2 vs_tile[];

out vec2 te_tex[];
out vec2 te_tile[];

void main() {

    
//     gl_TessLevelOuter[0] = tess_in[gl_InvocationID].x; // x
//     gl_TessLevelOuter[1] = tess_in[gl_InvocationID].y; // y
//     gl_TessLevelOuter[2] = tess_in[gl_InvocationID].x; // x 
//     gl_TessLevelOuter[3] = tess_in[gl_InvocationID].y; // y
//     
//     gl_TessLevelInner[0] = tess_in[gl_InvocationID].x;
//     gl_TessLevelInner[1] = tess_in[gl_InvocationID].y;
//     
//     gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
//     
	if(gl_InvocationID == 0) {
		gl_TessLevelOuter[0] = 64; 
		gl_TessLevelOuter[1] = 64; 
		gl_TessLevelOuter[2] = 64; 
		gl_TessLevelOuter[3] = 64; 
	
		gl_TessLevelInner[0] = 64;
		gl_TessLevelInner[1] = 64;
	}
		
	te_tex[gl_InvocationID] = vs_tex[gl_InvocationID];
	te_tile[gl_InvocationID] = vs_tile[gl_InvocationID];
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	
} 


#shader TESS_EVALUATION


#version 430 core

layout (quads, equal_spacing, ccw) in;

in vec2 te_tex[];
in vec2 te_tile[];

uniform sampler2D sHeightMap;


uniform mat4 mView;
uniform mat4 mProj;
uniform mat4 mModel;

uniform vec2 winSize;

const vec2 size = vec2(2,0.0);
const ivec3 off = ivec3(-1,0,1);



out vec2 texCoord;
out vec2 t_tile;
out vec4 te_normal;


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
	
	
	float t = texture2D(sHeightMap, ttmp.xy, 0).r;
	
	// normals. remember that z is still up at this point
	float xm1 = textureOffset(sHeightMap, ttmp.xy, off.xy).x;
	float xp1 = textureOffset(sHeightMap, ttmp.xy, off.zy).x;
	float ym1 = textureOffset(sHeightMap, ttmp.xy, off.yx).x;
	float yp1 = textureOffset(sHeightMap, ttmp.xy, off.yz).x;

	float sx = (xp1 - xm1);
	float sy = (yp1 - ym1);

	te_normal = normalize(vec4(sx*32, sy*32 ,1.0,1.0));
	

	tmp.z = t * .05; // .01 *  sin(gl_TessCoord.y*12) + .01 *sin(gl_TessCoord.x*12);

	gl_Position = (mProj * mView * mModel) * tmp;
	t_tile =  tltmp; 
	texCoord = ttmp; 
}


#shader FRAGMENT

 
#version 420



uniform vec2 cursorPos;
// uniform float cursorRad;


in vec2 texCoord;
in vec2 t_tile;
in vec4 te_normal;


const vec2 eye = vec2(500,500);

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;
layout(location = 2) out ivec4 out_Selection;

#define UNIT 1
#define HALFUNIT .5

uniform sampler2D sBaseTex;
uniform isampler2DArray sMap; // 0 = zones, 1 = surfaceTex
uniform sampler1D sZoneColors;

void main(void) {
	
	ivec2 tile = ivec2(floor(texCoord * 1024));
	
	int zoneIndex = texelFetch(sMap, ivec3(tile.xy,0), 0).r;
	vec4 zoneColor = texelFetch(sZoneColors, zoneIndex, 0);
	
	float scaleNear = 4;
	float scaleFar = 32;
	
	float q = mod(texCoord.x * 1024, scaleFar) / scaleFar;
	float r = mod(texCoord.y * 1024, scaleFar) / scaleFar;
	
	float qn = mod(texCoord.x * 1024, scaleNear) / scaleNear;
	float rn = mod(texCoord.y * 1024, scaleNear) / scaleNear;
	
	float d = distance(t_tile, cursorPos);
	if(d < 200) {
		float s = clamp((d - 190) / 10, 0.0, 1.0);
		q = mix(qn , q, s);
		r = mix(rn , r, s);
	}
	
	
	float ei1 = smoothstep(0.0, 0.01, q);
	float ei2 = 1.0 - smoothstep(0.99, 1.0, q);
	float ei3 = smoothstep(0.0, 0.01, r);
	float ei4 = 1.0 - smoothstep(0.99, 1.0, r);
	
	//out_Color = vec4(t_tile.x, t_tile.y,1 ,1.0);
	
 	
 	vec4 tc = texture2D(sBaseTex, texCoord);
//  	vec4 tc = vec4(texture(sMap, vec3(texCoord, 1)).rgb * 128, 1.0);
 	
 	bool incx = t_tile.x > cursorPos.x && t_tile.x < cursorPos.x + UNIT;
 	bool incy = t_tile.y > cursorPos.y && t_tile.y < cursorPos.y + UNIT;
 	
	//float distToCursor = length(gl_TessCoord.xy - cursorPos);
	vec4 cursorIntensity = (incx && incy ) ? vec4(0,10.0,10.0, 1.0) : vec4(1,1,1,1) ;//0 cursorRad - exp2(-1.0*distToCursor*distToCursor);
	
	out_Selection = ivec4(floor(t_tile.x), floor(t_tile.y) , 1, 1);
	out_Normal = vec4(te_normal.xyz, 1);
	out_Color =  (zoneColor * .2 + tc) * cursorIntensity * vec4(min(min(ei1, ei2), min(ei3, ei4)), 0,0,1).rrra; //(1.0, 0, .5, .6);
}

