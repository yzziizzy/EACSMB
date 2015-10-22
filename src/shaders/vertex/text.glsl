#version 430 core

layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec2 tex_in;
layout (location = 2) in vec4 color_in;

uniform mat4 mProj;
uniform mat4 mModel;


out vec2 texCoord;
out vec4 color;

void main() {
// 	gl_Position = (mProj * mView * mModel) * vec4(pos_in.xyz, 1.0);

	texCoord = tex_in;
	color = color_in;
	gl_Position = (mProj * mModel) * vec4(pos_in.xyz, 1.0);
}