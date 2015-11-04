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

const vec2 size = vec2(2,0.0);
const ivec3 off = ivec3(-1,0,1);



// out vec4 ex_Color;
out vec2 texCoord;
// out vec2 cursorTexCoord;
out vec2 t_tile;
out vec4 te_normal;


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
	
	
	float t = texture2D(sHeightMap, ttmp.xy, 0);
	
	// normals. remember that z is still up at this point
	float xm1 = textureOffset(sHeightMap, ttmp.xy, off.xy).x;
	float xp1 = textureOffset(sHeightMap, ttmp.xy, off.zy).x;
	float ym1 = textureOffset(sHeightMap, ttmp.xy, off.yx).x;
	float yp1 = textureOffset(sHeightMap, ttmp.xy, off.yz).x;

	float sx = (xp1 - xm1);
	float sy = (yp1 - ym1);

	te_normal = normalize(vec4(sx*32, sy*32 ,1.0,1.0));
	

	tmp.z = t * .05; // .01 *  sin(gl_TessCoord.y*12) + .01 *sin(gl_TessCoord.x*12);

	gl_Position = (mProj * mView * mModel) * tmp;
	t_tile =  tltmp; 
	texCoord = ttmp; 
}
