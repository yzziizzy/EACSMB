
#shader VERTEX


#version 430 core

layout(std140) uniform; 

// per vertex
layout (location = 0) in vec3 v_pos_in;
layout (location = 1) in vec2 v_tex_in;

// per instance
layout (location = 2) in vec4 i_center_radius_in;
layout (location = 3) in ivec2 i_texIndex_in;



uniform sampler2DArray sHeightMap;



uniform mat4 mWorldView;
uniform mat4 mViewProj;

out vec2 vs_tex;
flat out int vs_texIndex;


void main() {
	
	
	vec2 pos_ = v_pos_in.xy * (i_center_radius_in.w);
	vec4 pos = vec4(i_center_radius_in.xy + pos_.xy, v_pos_in.z + i_center_radius_in.z, 1.0);
	

	
	float h = texture(sHeightMap, vec3(pos.xy, 0)).r;
	//pos.z += h;
	
	gl_Position = (mViewProj * mWorldView) * pos;
	vs_tex = v_tex_in;
	vs_texIndex = int(i_texIndex_in.x);
}




#shader FRAGMENT

#version 400


uniform sampler2DArray sTextures;

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;


in vec2 vs_tex;
flat in int vs_texIndex;

void main(void) {
	
	
	vec4 t = texture(sTextures, vec3(vs_tex, vs_texIndex));
	
	
	out_Color = t;
	
}

