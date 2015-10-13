#version 430 core

layout (quads, equal_spacing, ccw) in;

uniform mat4 mView;
uniform mat4 mProj;
uniform mat4 mModel;

out vec4 ex_Color;

void main(void){

	vec4 p1 = mix(gl_in[1].gl_Position, gl_in[0].gl_Position, gl_TessCoord.x);
	vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
	vec4 tmp = mix(p1, p2, gl_TessCoord.y);
	
	tmp.z = sin(gl_TessCoord.y*12) + sin(gl_TessCoord.x*12);
	
	vec3 tang = gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz;
	vec3 bita = gl_in[0].gl_Position.xyz - gl_in[2].gl_Position.xyz;
	
	vec3 cnorm = cross(tang, bita);
	 
	vec4 norm = vec4(normalize(cnorm).xyz, 1.0);
	
	//tmp = tmp + (norm * sin(gl_TessCoord.y*4));
	
	gl_Position = (mProj * mView * mModel) * tmp;
	
	ex_Color = vec4(gl_TessCoord.x, gl_TessCoord.y, .3, 0);
}
