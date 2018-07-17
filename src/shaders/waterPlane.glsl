
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

/*
float gerstner(amp, wl, dir, phase, time, pos) {
	return amp * sin(
		dot(wl * dir, pos) + phase * time)
	);
	
}*/

// vec3 gerstner_normal(amp, wl, dir, phase, time, pos) {
// 	return vec3(
// 		amp * sin(dot(wl * dir, pos) + phase * time)),
// 	);
// 	
// }





void main(void) {
	
	float speed = 2.1; // radians per second 
	
	// TODO: move to uniform
	float theta = mod(timeSeconds * speed, 6.28) + (timeFractional * speed);
	
	vec3 color1 = vec3(.3, .3, .8);
	vec3 color2 = vec3(.6, .6, 1);
	
	out_Color = vec4(mix(color1, color2, (sin(vs_tex.x * 30 + theta) + 1) * .5), .5);
	
/*	out_Color = vec4(
		vs_tex.x,
		vs_tex.y,
		0, 1);
	*/
	float f = sin(vs_tex.x * 30 + theta) ;
	float fc = cos(vs_tex.x * 30 + theta);
// 	out_Color = vec4(vs_tex.xy, 0, 1); //vs_norm;
//	out_Color = vec4(1,0, 0, 1); //vs_norm;

	vec3 norm = normalize(vec3(fc,0, f));
	
	out_Normal = vec4((norm.xyz * .5) + .5, 1);
}

