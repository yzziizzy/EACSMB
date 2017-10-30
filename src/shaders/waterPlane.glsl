
#shader VERTEX


#version 430 core

layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec2 tex_in;


uniform mat4 mModel;
uniform mat4 mView;
uniform mat4 mProj;

// out vec4 vs_pos;
out vec4 vs_norm;
out vec2 vs_tex;

void main() {
	gl_Position = (mProj * mView * mModel) * vec4(pos_in, 1);
	vs_norm = vec4(0,1,0, 1); // normalize(vec4(1,1,1,0));
	vs_tex = tex_in;
}




#shader FRAGMENT

#version 400

// in vec4 vs_pos;
in vec4 vs_norm;
in vec2 vs_tex;

// fragment shader
uniform vec4 color;

uniform globalTimer {
	float timeSeconds;
	float timeFractional;
};


layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;


void main(void) {
	
	float speed = .1; // radians per second 
	
	// TODO: move to uniform
	float theta = mod(timeSeconds * speed, 6.28) + (timeFractional * speed);
	
	
	
// 	out_Color = vec4(vs_tex.xy, 0, 1); //vs_norm;
	out_Color = vec4(1,0, 0, 1); //vs_norm;
	out_Normal = normalize(vs_norm);
}

