#version 430 core

layout (location = 0) in vec3 pos_in;

uniform mat4 mView;
uniform mat4 mProj;
uniform mat4 mModel;


void main() {
// 	gl_Position = (mProj * mView * mModel) * vec4(pos_in.xyz, 1.0);
	gl_Position = vec4(pos_in.xyz, 1.0);
}