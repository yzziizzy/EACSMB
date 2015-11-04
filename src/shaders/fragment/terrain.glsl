 
#version 420


// fragment shader
uniform vec2 cursorPos;
// uniform float cursorRad;

// in vec4 ex_Color;
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
	out_Color =  zoneColor * tc * cursorIntensity * vec4(min(min(ei1, ei2), min(ei3, ei4)), 0,0,1).rrra; //(1.0, 0, .5, .6);
}