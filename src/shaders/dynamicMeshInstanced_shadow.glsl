
#shader VERTEX


#version 430 core

//layout(std140) uniform; 

// per-vertex attributes
layout (location = 0) in vec3 v_pos_in;
layout (location = 1) in vec3 v_norm_in;
layout (location = 2) in vec2 v_tex_in;


// per-instance attributes
// layout (location = 3) in vec4 i_pos_scale_in;
// layout (location = 4) in vec4 i_dir_rot_in;
// layout (location = 5) in vec4 i_alpha_in;

layout (location = 3) in mat4 i_mat_in;
layout (location = 7) in ivec2 i_tex_in;

uniform mat4 mWorldProj;

// out vec4 vs_pos;
out vec3 vs_norm;
out vec2 vs_tex;
out float vs_alpha;
flat out ivec2 vs_tex_indices;

void main() {
	vec4 pos = vec4(v_pos_in, 1.0);
	
	gl_Position = (mWorldProj * i_mat_in) * (pos);// * i_scale_in;
	vs_tex = v_tex_in;
	vs_tex_indices = i_tex_in;
}




#shader FRAGMENT

#version 400

// in vec4 vs_pos;
in vec3 vs_norm;
in vec2 vs_tex;
in float vs_alpha;
flat in ivec2 vs_tex_indices;

// fragment shader
uniform sampler2DArray sTexture;

// layout(location = 0) out vec4 out_Color;
// layout(location = 1) out vec4 out_Normal;
// 

void main(void) {

	/* // for performance
	vec4 tex = texture(sTexture, vec3(vs_tex.x, 1-vs_tex.y, vs_tex_indices.x));
	
	if(tex.a < 0.1) discard;
	*/
	
	// just write depth
}

