 
#version 400


// fragment shader

in vec4 ex_Color;
in vec2 texCoord;

out vec4 out_Color;

uniform sampler2D sHeightMap;

void main(void)
{
	vec4 tc = texture2D(sHeightMap, texCoord);
	out_Color = ex_Color; //(1.0, 0, .5, .6);
// 	out_Color = vec4(tc.rgb, 1.0); //ex_Color; //(1.0, 0, .5, .6);
}