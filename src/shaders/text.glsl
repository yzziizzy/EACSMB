
#shader VERTEX


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




#shader FRAGMENT

#version 400


// fragment shader
uniform sampler2D fontTex;

in vec2 texCoord;
in vec4 color;



void main(void) {
//  	gl_FragColor = vec4(texCoord.x, texCoord.y, .5, 0);// texture2D(fontTex, texCoord).rrrr;
	
	float alpha = texture2D(fontTex, texCoord).r;
	
	if(alpha < .01) {
		discard;
	}
	else {
		gl_FragColor = alpha * vec4(color.a, color.b, color.g, color.r); // probably a better way to do this. it's late, meh
		
	}
// 	out_Color = vec4(1.0, 0, .5, 0);
}

