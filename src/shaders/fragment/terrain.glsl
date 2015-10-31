 
#version 400


// fragment shader
uniform vec2 cursorPos;
// uniform float cursorRad;

in vec4 ex_Color;
in vec2 texCoord;
in vec2 t_tile;

out vec4 out_Color;


#define UNIT (1.0/1024.0)
#define HALFUNIT 20

uniform sampler2D sBaseTex;

void main(void)
{
	float q = mod(texCoord.x * 1024, 128);
	float r = mod(texCoord.y * 1024, 128);
	
 	float nearD = min(q,r);
// 	float nearD = mod(tile.x, 256);
// 	float edgeIntensity = exp2(-1.0*nearD*nearD);
	
	float edgeIntensity = 1.0 - exp2(-1.0*nearD*nearD);
	float edgeIntensity2 = exp2(-1.0*nearD*nearD);
	
	
	//out_Color = vec4(t_tile.x, t_tile.y,1 ,1.0);
	
 	
 	vec4 tc = texture2D(sBaseTex, texCoord);
 	
 	bool incx = t_tile.x > cursorPos.x - HALFUNIT && t_tile.x < cursorPos.x + HALFUNIT;
 	bool incy = t_tile.y > cursorPos.y - HALFUNIT && t_tile.y < cursorPos.y + HALFUNIT;
 	
	//float distToCursor = length(gl_TessCoord.xy - cursorPos);
	vec4 cursorIntensity = (incx && incy ) ? vec4(0,0,0, 1.0) : vec4(1,1,1,1) ;//0 cursorRad - exp2(-1.0*distToCursor*distToCursor);
 	
	out_Color = tc * cursorIntensity * max(edgeIntensity,edgeIntensity2); //(1.0, 0, .5, .6);
// 	out_Color = vec4(tc.rgb, 1.0); //ex_Color; //(1.0, 0, .5, .6);
	
}