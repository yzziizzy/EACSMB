#version 400


layout (location = 0) in vec3 pos;

uniform mat4 world;

void main() {
	gl_Position = world * vec4(pos, 1.0);
}




