
#shader VERTEX


#version 430 core

// per vertex
layout (location = 0) in vec3 v_pos_in;
layout (location = 1) in vec2 v_tex_in;

// per instance
layout (location = 2) in vec4 vi_pos1_thickness_in;
layout (location = 3) in vec4 vi_pos2_alpha_in;
layout (location = 4) in vec4 vi_pos3_unused_in;
layout (location = 5) in vec4 vi_pos4_unused_in;
layout (location = 6) in vec2 vi_tex_tile_in;

uniform mat4 mWorldView;
uniform mat4 mViewProj;

// out vec4 vs_pos;
out vec2 vs_tex;

flat out vec2 positions[4];

flat out float alpha;
flat out int texIndex;
flat out int tileInfo;

flat out float xp_yd;
flat out float xm_yd;
flat out float yp_xd;
flat out float ym_xd;

out float debug;

void main() {
	//vs_pos = v_pos_in.xyz;
	
	vec3 pos1 = vi_pos1_thickness_in.xyz;
	vec3 pos2 = vi_pos2_alpha_in.xyz;
	vec3 pos3 = vi_pos3_unused_in.xyz;
	vec3 pos4 = vi_pos4_unused_in.xyz;
	
	
	
	vec3 pos;
	int m = int(v_tex_in.x) % 4;
	int s = int(v_tex_in.x) / 4;
	
	//m = 4;
	
	debug = m;
	
	if(m == 0) pos = pos1;
	else if(m == 1) pos = pos2;
	else if(m == 2) pos = pos3;
	else pos = pos4;
	
	pos.z = s * vi_pos1_thickness_in.w;
	
	
	/*gl_Position = (mViewProj * mWorldView) * vec4(
		(v_pos_in.xy * 20) + vi_pos1_thickness_in.xy, 
		30 + vi_pos1_thickness_in.z, 
		1.0);*/
	gl_Position = (mViewProj * mWorldView) * vec4(
		pos.xy, 
		pos.z * vi_pos1_thickness_in.w,
		1.0);
	
	vs_tex = v_tex_in.xy;
	
	positions[0] = pos1.xy;
	positions[1] = pos2.xy;
	positions[2] = pos3.xy;
	positions[3] = pos4.xy;
	
	xp_yd = distance(pos3.xy, pos4.xy); // y-axis length on the +x side
	xm_yd = distance(pos1.xy, pos2.xy); // ...
	yp_xd = distance(pos2.xy, pos3.xy); 
	ym_xd = distance(pos1.xy, pos4.xy); 
	
}




#shader FRAGMENT

#version 400

// in vec4 vs_pos;
in vec2 vs_tex;

flat in vec2 positions[4];

flat in float alpha;
flat in int texIndex;
flat in int tileInfo;

flat in float xp_yd;
flat in float xm_yd;
flat in float yp_xd;
flat in float ym_xd;

in float debug;

// fragment shader
uniform vec2 screenSize;

uniform sampler2D sDepth;
uniform sampler2DArray sTexture;

uniform mat4 mWorldView;
uniform mat4 mViewProj;

// inverse matrices
uniform mat4 mViewWorld;
uniform mat4 mProjView;

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;



void main(void) {
	
	//out_Color=vec4(1,0,0, 1);
	//out_Normal=vec4(1,0,0,1);
	
	
	
	vec2 screenCoord = gl_FragCoord.xy / vec2(800,800);//screenSize;
	
	float depth = texture(sDepth, screenCoord).r;
	if (depth > 0.99999) {
		discard; // stencil later?
	}
	
	float ndc_depth = depth * 2.0 - 1.0;
	
	vec4 tmppos = inverse(mViewProj * mWorldView) * vec4(screenCoord * 2.0 - 1.0, ndc_depth, 1.0);
    vec3 pos = tmppos.xyz / tmppos.w;

	
	
    //vec3 npos = pos / 256;
// 	xp_yd = distance(pos3.xy, pos4.xy ); // y-axis length on the +x side
// 	xm_yd = distance(pos1.xy, pos2.xy ); // ...
// 	yp_xd = distance(pos2.xy, pos3.xy ); 
// 	ym_xd = distance(pos1.xy, pos4.xy ); 
	
	vec3 c;
// 	c.r = abs(positions[0].x - pos.x) / yp_xd;
	c.x = abs(positions[2].x - pos.x) / yp_xd;
	
	// 	c.y = abs(positions[0].y - pos.y) / xm_yd;
	
	//c.x  = distance(positions[0].xy, pos.xy);
	
	float dist = 1;// distance(pos.xy, vs_pos.xy);
	float hsize = 2 / 2;
	vec2 tc = vec2(0,0);//((pos.xy - vs_pos.xy) + hsize) ;
	
	//out_Color = vec4(npos.xy, 1, 1.0);
	
	
    //out_Color = vec4(0,1,1 , 1); //vs_norm;
	out_Color = vec4(c.x, 0, 0, 1);
// 	out_Color = //vec4(texture(sTexture, vec3(tc,0)).rgb , 1); //vs_norm;
	out_Normal = vec4(1,0,0,0);
	
	
}

