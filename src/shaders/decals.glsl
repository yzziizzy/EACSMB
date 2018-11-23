
#shader VERTEX


#version 430 core

// per vertex
layout (location = 0) in vec3 v_pos_in;
layout (location = 1) in vec2 v_tex_in;

// per instance
layout (location = 2) in vec4 v_pos_size_in;
layout (location = 3) in vec4 v_rot_alpha_lerp_in;
layout (location = 4) in ivec2 v_tex_tile_in;

uniform mat4 mWorldView;
uniform mat4 mViewProj;

// out vec4 vs_pos;
out vec2 vs_tex;

flat out vec3 vs_pos;
flat out float size;
flat out float rotation;
flat out float alpha;
flat out vec2 lerps;
flat out int texIndex;
flat out int tileInfo;



void main() {
	vs_pos = v_pos_size_in.xyz;
	size = v_pos_size_in.w;
	rotation = v_rot_alpha_lerp_in.x;
	lerps = v_rot_alpha_lerp_in.zw;
	alpha = v_rot_alpha_lerp_in.y;
	texIndex = v_tex_tile_in.x;
	tileInfo = v_tex_tile_in.y;

	gl_Position = (mViewProj * mWorldView) * vec4((v_pos_in * size) + v_pos_size_in.xyz, 1.0);
	vs_tex = v_tex_in.xy;
	
}




#shader FRAGMENT

#version 400

// in vec4 vs_pos;
in vec2 vs_tex;

flat in vec3 vs_pos; // center of the decal
flat in float size;
flat in float rotation;
flat in float alpha;
flat in vec2 lerps;
flat in int texIndex;
flat in int tileInfo;


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



void main(void) {
	
	//out_Color=vec4(1,0,0, 1);
	//out_Normal=vec4(1,0,0,1);
	
	
	
	vec2 screenCoord = gl_FragCoord.xy / targetSize;// vec2(800,800);//screenSize;
	
	float depth = texture(sDepth, screenCoord).r;
	if (depth > 0.99999) {
		discard; // stencil later?
	}
	
	float ndc_depth = depth * 2.0 - 1.0;
	
	vec4 tmppos = inverse(mViewProj * mWorldView) * vec4(screenCoord * 2.0 - 1.0, ndc_depth, 1.0);
    vec3 pos = tmppos.xyz / tmppos.w;

    
    //vec3 npos = pos / 256;
    
	float dist = distance(pos.xy, vs_pos.xy);
	float hsize = size / 2;
	vec2 tc = ((pos.xy - vs_pos.xy) + hsize) / size;
	
	//out_Color = vec4(pos.xy, 1, 1.0);
	
	
	if(dist > hsize) {
		//out_Color = vec4(1,0,dist / 5,1);
		discard; // commented for debug
	}
    else {
		//out_Color = vec4(0,1,1 , 1); //vs_norm;
		//float blend = clamp((hsize - dist) + lerps.x, 0, 1);
		out_Color = vec4(texture(sTexture, vec3(tc, texIndex)).rgb, 1); //vs_norm;
		out_Normal = vec4(1,0,0,0);
	}
	
}

