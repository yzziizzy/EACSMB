
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
layout (location = 7) in ivec4 i_tex_in;

uniform mat4 mWorldView;
uniform mat4 mViewProj;

// out vec4 vs_pos;
out vec3 vs_norm;
out vec2 vs_tex;
out float vs_alpha;
flat out ivec4 vs_tex_indices;


void main() {
	vec4 pos = vec4(v_pos_in, 1.0);
//	pos *= rotationMatrix(i_dir_rot_in.xyz, i_dir_rot_in.w);
//	pos *= vec4(i_pos_scale_in.www, 1);
//	pos += vec4(i_pos_scale_in.xyz, 0);
	
	gl_Position = (mViewProj * mWorldView * i_mat_in) * (pos);// * i_scale_in;

	// erase translation from the matrix.
	mat4 norm_mat = i_mat_in;
	norm_mat[3][0] = 0;
	norm_mat[3][1] = 0;
	norm_mat[3][2] = 0;
	
// 	vs_norm = normalize((i_mat_in * vec4(v_norm_in, 1.0)).xyz);
	vs_norm = normalize((norm_mat * vec4(v_norm_in, 1.0)).xyz);

	vs_tex = vec2(v_tex_in.x, 1.0 - v_tex_in.y);
	vs_alpha = 1.0;//i_alpha_in.x;
	vs_tex_indices = i_tex_in;
}




#shader FRAGMENT

#version 400

// in vec4 vs_pos;
in vec3 vs_norm;
in vec2 vs_tex;
in float vs_alpha;
flat in ivec4 vs_tex_indices;

// fragment shader
uniform sampler2DArray sTexture; // RGBA
uniform sampler2DArray sNormalTextures; // RGB
uniform sampler2DArray sMaterialTextures; // R


uniform mat4 mWorldView;
uniform mat4 mViewProj;

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;
layout(location = 2) out vec3 out_Material; // metal, rough, 


void main(void) {

	vec4 tex = texture(sTexture, vec3(vs_tex.xy, vs_tex_indices.x));
	
	if(tex.a < 0.1) discard;

	vec4 norm = texture(sNormalTextures, vec3(vs_tex.xy, vs_tex_indices.y));
	float metallic = texture(sMaterialTextures, vec3(vs_tex.xy, vs_tex_indices.z)).r;
	float roughness = texture(sMaterialTextures, vec3(vs_tex.xy, vs_tex_indices.w)).r;

	// TODO: blend the normals properly
	
	out_Color = vec4(tex.rgba); 
 	out_Normal = vec4((vs_norm.xyz * .5) + .5, 1.0); // alpha is roughness
	out_Material = vec3(metallic, roughness, 1);
}

