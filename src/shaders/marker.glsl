
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
flat out float vs_divisor;

void main() {
	
	
	vec2 pos_ = v_pos_in.xy * (i_center_radius_in.w);
	vec4 pos = vec4(i_center_radius_in.xy + pos_.xy, v_pos_in.z + i_center_radius_in.z, 1.0);
	

	
	float h = texture(sHeightMap, vec3(pos.xy, 0)).r;
	//pos.z += h;
	
	gl_Position = (mViewProj * mWorldView) * pos;
	vs_tex = v_tex_in;
	vs_texIndex = int(i_texIndex_in.x);
	vs_divisor = i_texIndex_in.y;
}




#shader FRAGMENT

#version 400


uniform sampler2DArray sTextures;
uniform float timeFractional;

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;
layout(location = 2) out vec3 out_Material; // metal, rough, 


in vec2 vs_tex;
flat in int vs_texIndex;
flat in float vs_divisor;



void main(void) {
	
	vec2 tex = vec2(vs_tex.x * vs_divisor + timeFractional, vs_tex.y); 
	vec2 tex2 = vec2(vs_tex.x * vs_divisor + (1 - timeFractional), vs_tex.y); 
	
	vec4 t = texture(sTextures, vec3(tex, vs_texIndex));
	vec4 t2 = texture(sTextures, vec3(tex2, vs_texIndex));
	
	vec4 t3 = (t + t2) / 2;
	
	if(t3.a < 0.01) discard;
	
	out_Color = t3;
	out_Material = vec3(0, 0.5, 1);
}

