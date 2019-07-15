
#include "include/lighting_pbr.glsl"

#shader VERTEX


#version 430 core

//layout(std140) uniform; 

// per-vertex attributes
layout (location = 0) in vec4 v_pos_type_in;


// per-instance attributes
layout (location = 1) in vec4 v_center_const_in;
layout (location = 2) in vec4 v_dir_lin_in;
layout (location = 3) in vec4 v_color_quad_in;
layout (location = 4) in vec4 v_cut_exp_radius_in;



uniform mat4 mWorldView;
uniform mat4 mViewProj;

out vec3 vs_center;
out float vs_constant;
out float vs_linear;
out float vs_quadratic;
out float vs_radius;
out float vs_cutoff;
out float vs_exponent;


void main() {
	vec4 pos = vec4(v_pos_type_in.xyz, 1.0);
	vec4 center = vec4(v_center_const_in.xyz, 0.0);
//	pos *= rotationMatrix(i_dir_rot_in.xyz, i_dir_rot_in.w);
//	pos *= vec4(i_pos_scale_in.www, 1);
	pos *= vec4(v_cut_exp_radius_in.zzz, 1.0); 
	pos += vec4(v_center_const_in.xyz, 0);
	
	gl_Position = (mViewProj * mWorldView) * (pos);
	vs_center = center.xyz;
	vs_constant = v_center_const_in.w;
	vs_linear = v_dir_lin_in.w;
	vs_quadratic = v_color_quad_in.w;
	vs_cutoff = v_cut_exp_radius_in.x;
	vs_exponent = v_cut_exp_radius_in.y;
	vs_radius = v_cut_exp_radius_in.z;
	
	vs_constant = .6;
	vs_linear = .12;
	vs_quadratic = .008;
}




#shader FRAGMENT

#version 400

// in vec4 vs_pos;


// fragment shader
// uniform vec4 color;
// uniform sampler2D sTexture;

uniform sampler2D sDiffuse;
uniform sampler2D sNormals;
uniform sampler2D sDepth;


uniform mat4 mProjWorld;
uniform vec2 screenSize;
// temp
uniform mat4 mWorldView;
uniform mat4 mViewProj;

uniform ivec2 targetSize;

layout(location = 0) out vec4 out_Light;


in vec3 vs_center;
in float vs_constant;
in float vs_linear;
in float vs_quadratic;
in float vs_radius;
in float vs_cutoff;
in float vs_exponent;


void main(void) {
	
	vec2 screenCoord = gl_FragCoord.xy / targetSize;//screenSize;
	
	float depth = texture(sDepth, screenCoord).r;
	if (depth > 0.99999) {
		discard; // stencil later?
	}
	
	float ndc_depth = depth * 2.0 - 1.0;
	
	vec4 tmppos = inverse(mViewProj * mWorldView) * vec4(screenCoord * 2.0 - 1.0, ndc_depth, 1.0);
	vec3 pos = tmppos.xyz / tmppos.w;
	//------------------
	
	float d = distance(pos, vs_center);
	if(d > vs_radius) {
		discard;
	}
	
	
	
	vec2 tex = gl_FragCoord.xy / targetSize.xy;
	vec4 raw_normal = texture(sNormals, tex);
	vec4 raw_diffuse = texture(sDiffuse, tex);
	vec3 normal = (raw_normal.xyz * 2.0) - 1.0;
	vec3 diffuseColor = raw_diffuse.rgb;

	if(raw_normal.xyz != vec3(0,0,0)) normal = normalize(normal);

	vec3 viewpos = (inverse(mViewProj * mWorldView) * vec4(0,0,0,1)).xyz;
	vec3 viewdir = normalize((inverse(mViewProj * mWorldView) * vec4(0,0,1,1)).xyz);
	
	// world space
// 	vec3 ldir = normalize(pos - vs_center);
	vec3 ldir = normalize(vs_center - pos);
	vec3 h = normalize(ldir + viewdir);
	
	float att = 1.0 / (vs_constant + vs_linear * d + vs_quadratic * d * d);
	
// 	out_Light = vec4(att, att, att, 1.0);
	
	vec4 o = att * 5 * vec4(f_Schlick_Smith_GGX(normal, h, ldir, viewdir, diffuseColor, .08, .61), 1.0);
	out_Light = clamp(o, 0, 2000);
// 	out_Light = vec4(.2, 1.0, .8, 1.0);
// 	out_Light = vec4(diffuseColor,1);
//  	out_Light = vec4(viewdir * .5 + .5,1);
}

