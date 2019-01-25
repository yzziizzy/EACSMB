
#shader VERTEX


#version 430 core

layout(std140) uniform; 

// per vertex
layout (location = 0) in vec3 pos_in;

uniform mat4 mWorldView;
uniform mat4 mViewProj;
uniform vec3 pMin;
uniform vec3 pMax;
uniform vec3 color;

flat out vec3 vs_color;

void main() {
	
	vec4 pos = vec4(
		pMin.x * (1 - pos_in.x) + pMax.x * pos_in.x,
		pMin.y * (1 - pos_in.y) + pMax.y * pos_in.y,
		pMin.z * (1 - pos_in.z) + pMax.z * pos_in.z,
		1.0);
	

 	gl_Position = (mViewProj * mWorldView) * pos;
// 	gl_Position =  pos;
	vs_color = color;
}




#shader FRAGMENT

#version 400

layout(location = 0) out vec4 out_Color;

flat in vec3 vs_color;

uniform sampler2D sDepth;




void main(void) {
	
	float d = texelFetch(sDepth, ivec2(gl_FragCoord.xy), 0).r;
	
	float a = 1.0;
	float o = 0.0;
	if(d <= gl_FragCoord.z) {
		a = .7;
		o = .4;
	}
	
	out_Color = vec4(vs_color - o, a);
// 	out_Color = vec4(1,a,0, a);
}

