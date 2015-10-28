 
#version 400


// fragment shader

in vec4 ex_Color;
in vec2 texCoord;
in vec2 t_tile;

out vec4 out_Color;


// uniform sampler2D sHeightMap;

void main(void)
{
	float q = mod(t_tile.x * 1024, 128);
	float r = mod(t_tile.y * 1024, 128);
	
 	float nearD = min(q,r);
// 	float nearD = mod(tile.x, 256);
// 	float edgeIntensity = exp2(-1.0*nearD*nearD);
	
	float edgeIntensity = 1.0 - exp2(-1.0*nearD*nearD);
	
	//out_Color = vec4(t_tile.x, t_tile.y,1 ,1.0);
	
// 	vec4 tc = texture2D(sHeightMap, texCoord);
	out_Color = ex_Color * edgeIntensity; //(1.0, 0, .5, .6);
// 	out_Color = vec4(tc.rgb, 1.0); //ex_Color; //(1.0, 0, .5, .6);
	
}