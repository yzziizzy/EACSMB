
#shader VERTEX


#version 430 core

layout (location = 0) in vec4 pos_in;


uniform mat4 mMVP;


void main() {
	gl_Position = (mMVP) * pos_in;
}




#shader FRAGMENT

#version 400


// fragment shader
uniform vec4 color;


layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;


void main(void) {
	
	out_Color = color;
	
}

