#version 430 core

layout (location = 0) in vec3 pos_in;
layout (location = 1) in ivec2 tex_in;
layout (location = 2) in ivec2 tess_in;


// out vec3 pos_out;
// out ivec2 tex_out;
//out ivec2 tess_out;

void main() {
// 	gl_Position = (mProj * mView * mModel) * vec4(pos_in.xyz, 1.0);
	gl_Position = vec4(pos_in.xyz, 1.0);
	
// 	pos_out = vec4(pos_in.xyz, 1.0);
// 	tex_out = tex_in;
//	tess_out = tess_in;
	
}