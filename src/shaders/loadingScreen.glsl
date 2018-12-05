#shader VERTEX


#version 430 core

layout (location = 0) in vec2 pos;


void main() {
	gl_Position = vec4(pos, 0.0, 1.0);
}



#shader FRAGMENT

#version 430 core

uniform float timer;
uniform vec2 resolution;

out vec4 FragColor;

void main() {
	
	vec2 tex = gl_FragCoord.xy / resolution.xy;
	
	FragColor = vec4(
		tex.x * cos(timer) * .5 + 1,
		tex.y * sin(timer) * .5 + 1,
		0, 1.0);
// 	FragColor = vec4(tex, 0, 1.0);
}









