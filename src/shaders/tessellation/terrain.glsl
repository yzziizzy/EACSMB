#version 430 core

layout (quads, equal_spacing, ccw) in;

// in vec4 pos_in[];
in vec2 te_tex[];
in vec2 te_tile[];

uniform sampler2D sHeightMap;

uniform mat4 mView;
uniform mat4 mProj;
uniform mat4 mModel;

uniform vec2 winSize;




out vec4 ex_Color;
out vec2 texCoord;
// out vec2 cursorTexCoord;
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
	
	
// 	vec2 u = mix(te_tex[1],  
	
 	float t = texture2D(sHeightMap, ttmp.xy, 0);
	
	tmp.z = t * .05; // .01 *  sin(gl_TessCoord.y*12) + .01 *sin(gl_TessCoord.x*12);
	
	vec3 tang = gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz;
	vec3 bita = gl_in[0].gl_Position.xyz - gl_in[2].gl_Position.xyz;
	
	vec3 cnorm = cross(tang, bita);
	 
	vec4 norm = vec4(normalize(cnorm).xyz, 1.0);
	
	//tmp = tmp + (norm * sin(gl_TessCoord.y*4));
	
	gl_Position = (mProj * mView * mModel) * tmp;
	
	//gl_in[0].gl_Position.xy/gl_PositionIn[0].w
	
	//dist = vec3(0.0, 0.0, sin(gl_TessCoord.y * 10));
	
	t_tile =  tltmp; //te_tile[3];
	
//  	ex_Color = vec4(gl_TessCoord.x, gl_TessCoord.y, .3, 1.0);
	ex_Color = vec4(ttmp.xy, .3, 1.0);
	texCoord = ttmp; //gl_TessCoord.xy;
	
	
	
}
