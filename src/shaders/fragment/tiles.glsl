 
#version 400


// fragment shader

in vec4 ex_Color;
out vec4 out_Color;



void main(void)
{
	out_Color = ex_Color; //vec4(1.0, 0, .5, 0);
}