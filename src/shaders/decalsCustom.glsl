
#shader VERTEX


#version 430 core

// per vertex
layout (location = 0) in vec3 v_pos_in;
layout (location = 1) in ivec2 v_tex_in;

// per instance
layout (location = 2) in vec4 vi_pos1_thickness_in;
layout (location = 3) in vec4 vi_pos2_alpha_in;
layout (location = 4) in vec4 vi_pos3_tex12_in;
layout (location = 5) in vec4 vi_pos4_tex34_in;
layout (location = 6) in ivec2 vi_tex_tile_in;

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

flat out vec2 texOffset;

out float debug;

void main() {
	//vs_pos = v_pos_in.xyz;
	
	vec3 pos1 = vi_pos1_thickness_in.xyz;
	vec3 pos2 = vi_pos2_alpha_in.xyz;
	vec3 pos3 = vi_pos3_tex12_in.xyz;
	vec3 pos4 = vi_pos4_tex34_in.xyz;
	
	
	
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
	yp_xd = distance(pos2.xy, pos4.xy); 
	ym_xd = distance(pos1.xy, pos3.xy); 
	
	texOffset = vec2(vi_pos3_tex12_in.w, vi_pos4_tex34_in.w);
	texIndex = vi_tex_tile_in.x;
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

flat in vec2 texOffset;

in float debug;

// fragment shader
uniform ivec2 targetSize;

uniform sampler2D sDepth;
uniform sampler2DArray sTexture;

uniform mat4 mWorldView;
uniform mat4 mViewProj;

// inverse matrices
uniform mat4 mViewWorld;
uniform mat4 mProjView;

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;


vec2 lineProject(vec2 a, vec2 b, vec2 p) {
	vec2 ab = b - a;
	vec2 ap = p - a;
	
	return a + dot(ap, ab) / dot(ab, ab) * ab;
}

vec2 lineInterp(vec2 a, vec2 b, vec2 p, out vec2 ip) {
	
	float d = distance(a, b);
	ip = lineProject(a, b, p);
	
	return vec2(
		(distance(ip, a) / d),
		(distance(ip, b) / d)
	);
}



void main(void) {
	
	//out_Color=vec4(1,0,0, 1);
	//out_Normal=vec4(1,0,0,1);
	
	
	
	vec2 screenCoord = gl_FragCoord.xy / targetSize;//screenSize;
	
	float depth = texture(sDepth, screenCoord).r;
	if (depth > 0.99999) {
		discard; // stencil later?
	}
	
	float ndc_depth = depth * 2.0 - 1.0;
	
	vec4 tmppos = inverse(mViewProj * mWorldView) * vec4(screenCoord * 2.0 - 1.0, ndc_depth, 1.0);
    vec3 pos = tmppos.xyz / tmppos.w;

	// end of position reconstruction

	// projected texture coordinates
	vec2 tc;
	
	// interpolated points
	vec2 ip_01, ip_23, ip_02, ip_13;
	
	// TODO: move as much as possible to the vertex shader
	
	// normalized positions
	vec2 np_01 = lineInterp(positions[0], positions[1], pos.xy, ip_01);
	vec2 np_23 = lineInterp(positions[2], positions[3], pos.xy, ip_23);
	
	vec2 np_02 = lineInterp(positions[0], positions[2], pos.xy, ip_02);
	vec2 np_13 = lineInterp(positions[1], positions[3], pos.xy, ip_13);
	
	float d_xip = distance(ip_01, ip_23);
	float r_x = distance(ip_01, pos.xy) / d_xip;
	
	float d_yip = distance(ip_02, ip_13);
	float r_y = distance(ip_02, pos.xy) / d_yip;
	
	// used to calculate bounds
	float r_ox = distance(ip_23, pos.xy) / d_xip;
	float r_oy = distance(ip_13, pos.xy) / d_yip;
	
	tc = vec2(r_y, r_x); // backwards for better intuitive orientation
	
	// discard the box
	if(max(max(tc.x, tc.y), max(r_oy, r_ox)) > 1 || min(tc.x, tc.y) < 0) {
	//	out_Color = vec4(1,0,0, .5); return;
		discard; 
	}
	
	// handle texture offset, wrap, and repeat
	vec2 otc = vec2(tc.x, mix(texOffset.x, texOffset.y, tc.y));
	
	float metallic = .1;//texture(sMaterialTextures, vec3(tc, texIndex)).r;
	float roughness = .71;//texture(sMaterialTextures, vec3(tc, texIndex)).r;
	
	//out_Color = vec4(mod(otc.x , 1), mod(otc.y, 1) ,0 , 1); //vs_norm;
	out_Color = vec4(texture(sTexture, vec3(otc, texIndex)).rgba); //vs_norm;
	out_Normal = vec4(1,0,0,roughness);
	
	//out_Color = vec4(1,0,1,1);
}

