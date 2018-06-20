
#shader VERTEX


#version 400 core

layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec2 tex_in;
layout (location = 2) in vec4 color_in;

uniform vec2 world; // terrible name. this is the position of the text as a whole.
uniform mat4 mProj;
uniform mat4 mModel;


out vec2 texCoord;
out vec4 color;

void main() {
// 	gl_Position = (mProj * mView * mModel) * vec4(pos_in.xyz, 1.0);
	
	
	
	texCoord = tex_in;
	color = color_in;
	gl_Position = mProj * ((vec4(pos_in.xyz, 1.0) * mModel) + vec4(world, 0.0, 0.0));
}




#shader FRAGMENT

#version 400


// fragment shader
uniform sampler2D fontTex;

in vec2 texCoord;
in vec4 color;



void main(void) {

	float d = texture2D(fontTex, texCoord).r;
	
	float a;
	
	if(d > .75) {
		d = 1;// (d - .75) * -4;
	}
	else {
		d = (d / 3) * 4;
	}
 	d = 1 - d;

 	a = smoothstep(0.2, 1.0, abs(d));
	
//	a = mix(0.1, .9, abs(d));

	//a = d;
	
	if(a < 0.01) discard;
	
	gl_FragColor = vec4(color.abg, a); 

}

