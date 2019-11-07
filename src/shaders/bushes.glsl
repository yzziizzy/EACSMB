
#shader VERTEX


#version 430

layout(std140) uniform; 

// per vertex
layout (location = 0) in vec3 v_pos_in;
layout (location = 0) in vec3 v_norm_in;
layout (location = 1) in vec2 v_tex_in;

// per instance
layout (location = 2) in vec4 i_pos_rot_in;
layout (location = 3) in ivec4 i_texIndex_in; // diffuse, normal, metallic, roughness




uniform mat4 mWorldView;
uniform mat4 mWorldProj;
uniform mat4 mViewProj;


out vec3 vs_norm;
out vec2 vs_tex;
flat out ivec4 vs_tex_indices;
flat out float vs_divisor;

void main() {
	
	
	vec4 pos = vec4(v_pos_in.xyz + i_pos_rot_in.xyz, 1.0);
	
	gl_Position = (mWorldProj) * pos;
	vs_norm = v_norm_in; // TODO: rotate with the rest
	vs_tex = v_tex_in;
	vs_tex_indices = i_texIndex_in;
}




#shader FRAGMENT

#version 430

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;
layout(location = 2) out vec3 out_Material; // metal, rough, 

uniform sampler2DArray sTextures; // RGBA
uniform sampler2DArray sMaterials; // R
uniform float timeFractional;



in vec2 vs_tex;
in vec3 vs_norm;
flat in ivec4 vs_tex_indices;



void main(void) {
	
	vec4 tex_color = texture(sTextures, vec3(vs_tex.xy, vs_tex_indices.x));
	vec3 tex_normal = texture(sTextures, vec3(vs_tex.xy, vs_tex_indices.y)).xyz;
	float roughness = texture(sMaterials, vec3(vs_tex.xy, vs_tex_indices.z)).r;
	
	
	
	float metalness = 0.05;
	
	
	out_Color = vec4(tex_color.xyz, 1);
	
	// normals need to be normalized and in world space
	vec3 out_norm = normalize(vs_norm + tex_normal); // BUG probably the wrong way to do this
	out_Normal = vec4((out_norm.xyz * .5) + .5, 1);
	
	out_Material = vec3(metalness, roughness, 1);
}

