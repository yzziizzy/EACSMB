#version 430 core

layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec2 tex_in;
layout (location = 2) in ivec2 tess_in;


// out vec3 pos_out;
out vec2 vs_tex;
// out ivec2 tess_out;

void main() {
// 	gl_Position = (mProj * mView * mModel) * vec4(pos_in.xyz, 1.0);
	
// 	pos_out = vec4(pos_in.xyz, 1.0);
	vs_tex = tex_in;
// 	tess_out = tess_in;
	
	gl_Position = vec4(pos_in.xyz, 1.0);

}