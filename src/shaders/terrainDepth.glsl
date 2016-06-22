

#shader VERTEX

#version 430 core

layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec2 tex_in;
layout (location = 2) in vec2 tile_in;

uniform sampler2D sOffsetLookup;

out vec2 vs_tex;
out vec2 vs_tile;
out int vs_InstanceID;

void main() {
	vs_tex = tex_in;
	vs_tile = tile_in;
	vs_InstanceID = gl_InstanceID;
	vec4 off = texelFetch(sOffsetLookup, ivec2(gl_InstanceID, 0), 0); 
	gl_Position = vec4(pos_in.x + (off.r * 255), pos_in.y + (off.g * 255), pos_in.z, 1.0);
}



#shader TESS_CONTROL

#version 430 core


layout (vertices = 4) out;




in vec2 vs_tex[];
in vec2 vs_tile[];
in int vs_InstanceID[];

out vec2 te_tex[];
out vec2 te_tile[];;
out int te_InstanceID[];

void main() {

    
//     gl_TessLevelOuter[0] = tess_in[gl_InvocationID].x; // x
//     gl_TessLevelOuter[1] = tess_in[gl_InvocationID].y; // y
//     gl_TessLevelOuter[2] = tess_in[gl_InvocationID].x; // x 
//     gl_TessLevelOuter[3] = tess_in[gl_InvocationID].y; // y
//     
//     gl_TessLevelInner[0] = tess_in[gl_InvocationID].x;
//     gl_TessLevelInner[1] = tess_in[gl_InvocationID].y;
//     
//     gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
//     
	if(gl_InvocationID == 0) {
		gl_TessLevelOuter[0] = 64; 
		gl_TessLevelOuter[1] = 64; 
		gl_TessLevelOuter[2] = 64; 
		gl_TessLevelOuter[3] = 64; 
	
		gl_TessLevelInner[0] = 64;
		gl_TessLevelInner[1] = 64;
	}
	
	te_tile[gl_InvocationID] = vs_tile[gl_InvocationID];
	te_tex[gl_InvocationID] = vs_tex[gl_InvocationID];
	te_InstanceID[gl_InvocationID] = vs_InstanceID[gl_InvocationID];
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	
} 


#shader TESS_EVALUATION


#version 430 core

layout (quads, equal_spacing, ccw) in;

in vec2 te_tile[];
in vec2 te_tex[];
in int te_InstanceID[];

uniform sampler2DArray sHeightMap;


uniform mat4 mView;
uniform mat4 mProj;
uniform mat4 mModel;


out vec2 t_tile;



void main(void){

	vec4 p1 = mix(gl_in[1].gl_Position, gl_in[0].gl_Position, gl_TessCoord.x);
	vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
	vec4 tmp = mix(p1, p2, gl_TessCoord.y);
	
	vec2 tp1 = mix(te_tex[1], te_tex[0], gl_TessCoord.x);
	vec2 tp2 = mix(te_tex[2], te_tex[3], gl_TessCoord.x);
	vec2 ttmp = mix(tp1, tp2, gl_TessCoord.y);
	
	vec2 tlp1 = mix(te_tile[1], te_tile[0], gl_TessCoord.x);
	vec2 tlp2 = mix(te_tile[2], te_tile[3], gl_TessCoord.x);
	vec2 tltmp = mix(tlp1, tlp2, gl_TessCoord.y);
	
	
	float t = texture(sHeightMap, vec3(ttmp.xy, te_InstanceID[0]), 0).r;
	
	tmp.z = t / 256; //* .05; // .01 *  sin(gl_TessCoord.y*12) + .01 *sin(gl_TessCoord.x*12);

	gl_Position = (mProj * mView * mModel) * tmp;
	t_tile =  tltmp; 
}


#shader FRAGMENT

 
#version 420




in vec2 t_tile;

layout(location = 2) out ivec4 out_Selection;



void main(void) {
	
	out_Selection = ivec4(floor(t_tile.x) , floor(t_tile.y)  , 1, 1);
}

