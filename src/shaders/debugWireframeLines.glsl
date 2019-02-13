
#shader VERTEX


#version 430 core

layout(std140) uniform; 

// per vertex
layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec4 color_in;

uniform mat4 mWorldView;
uniform mat4 mViewProj;

out vec4 vs_color;

void main() {
	

 	gl_Position = (mViewProj * mWorldView) * vec4(pos_in, 1.0);
// 	gl_Position =  pos;
	vs_color = color_in;
}




#shader FRAGMENT

#version 400

layout(location = 0) out vec4 out_Color;

in vec4 vs_color;

uniform sampler2D sDepth;




void main(void) {
	
	float d = texelFetch(sDepth, ivec2(gl_FragCoord.xy), 0).r;
	
	float a = 1.0;
	float o = 0.0;
	if(d <= gl_FragCoord.z) {
		a = .7;
		o = .4;
	}
	
	out_Color = vec4(vs_color.xyz - o, a);
// 	out_Color = vec4(1,a,0, 1);
}

