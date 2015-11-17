
#shader VERTEX


#version 430 core

layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec2 tex_in;



uniform mat4 mMVP;


out vec2 texCoord;

void main() {
// 	gl_Position = (mProj * mView * mModel) * vec4(pos_in.xyz, 1.0);

	texCoord = tex_in;
	gl_Position = (mMVP) * vec4(pos_in.xyz, 1.0);
}




#shader FRAGMENT

#version 400


// fragment shader
uniform sampler2DArray sTexture;
uniform int texIndex; 
uniform float glow; 

in vec2 texCoord;

layout(location = 0) out vec4 out_Color;


void main(void) {
	
	vec4 tc = texture(sTexture, vec3(texCoord.xy, texIndex)).rgba;
	
	out_Color = tc + vec4(glow, glow, glow, 1);
	
}

