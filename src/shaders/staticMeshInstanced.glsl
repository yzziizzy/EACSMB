
#shader VERTEX


#version 430 core

//layout(std140) uniform; 

// per-vertex attributes
layout (location = 0) in vec4 v_pos_in;
layout (location = 1) in vec4 v_norm_in;
layout (location = 2) in vec2 v_tex_in;


// per-instance attributes
layout (location = 3) in vec3 i_pos_in;
layout (location = 4) in vec3 i_dir_in;
layout (location = 5) in vec3 i_scale_in;

uniform mat4 mModel;
uniform mat4 mView;
uniform mat4 mProj;

// out vec4 vs_pos;
out vec4 vs_norm;
out vec2 vs_tex;

mat4 rotationMatrix(vec3 axis, float angle) {
	axis = normalize(axis);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;
	
	return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
				oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
				oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
				0.0,                                0.0,                                0.0,                                1.0);
}


void main() {
	vec4 pos = v_pos_in;
	pos *= rotationMatrix(i_dir_in, 3.14/2);
	pos += vec4(i_pos_in, 0);
	pos *= vec4(i_scale_in, 1);
	
	gl_Position = (mProj * mView * mModel) * (pos);// * i_scale_in;
	vs_norm = v_norm_in; // normalize(vec4(1,1,1,0));
	vs_tex = v_tex_in;
}




#shader FRAGMENT

#version 400

// in vec4 vs_pos;
in vec4 vs_norm;
in vec2 vs_tex;

// fragment shader
uniform vec4 color;
uniform sampler2D sTexture;

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;


void main(void) {
	
	out_Color = texture(sTexture, vec2(vs_tex.x, 1-vs_tex.y)); //vs_norm;
	out_Normal = vs_norm;
}

