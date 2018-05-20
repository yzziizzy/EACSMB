
#shader VERTEX


#version 430 core

//layout(std140) uniform; 

// per-vertex attributes
layout (location = 0) in vec3 v_pos_type_in;


// per-instance attributes
layout (location = 1) in vec3 v_center_const_in;
layout (location = 2) in vec3 v_dir_lin_in;
layout (location = 3) in vec2 v_color_quad_in;
layout (location = 4) in vec2 v_cut_exp_in;



uniform mat4 mWorldView;
uniform mat4 mViewProj;

// out vec4 vs_pos;


void main() {
	vec4 pos = vec4(v_pos_type_in.xyz, 1.0);
	vec4 center = vec4(v_center_const_in.xyz, 0.0);
//	pos *= rotationMatrix(i_dir_rot_in.xyz, i_dir_rot_in.w);
//	pos *= vec4(i_pos_scale_in.www, 1);
//	pos += vec4(i_pos_scale_in.xyz, 0);
	
	gl_Position = (mViewProj * mWorldView) * (pos * vec4(50, 50, 50, 1.0));
}




#shader FRAGMENT

#version 400

// in vec4 vs_pos;


// fragment shader
// uniform vec4 color;
// uniform sampler2D sTexture;

layout(location = 0) out vec4 out_Light;


void main(void) {
	
	out_Light = vec4(.2, 1.0, .8, 1.0);
}

