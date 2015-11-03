 
#version 430 core

// geometry shader 
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform sampler2D sZoneMap;


in vec2 texCoordX[];
in vec2 t_tileX[];
in vec4 te_normalX[];


// 	vec4 ex_Color;
out vec2 texCoord;
out vec2 t_tile;
out vec4 te_normal;
// out vec4 zoneColor;


void main(void) {
	
	
// 	ivec2 center = ivec2(floor(((t_tileX[0].xy + t_tileX[1].xy + t_tileX[2].xy) / 3).xy)); 
	
// 	vec4 zc = vec4(1,1,1,1);
	
// 	if(center.x % 2 == 1) {
// // 		zc = vec4(1,0,0,1);
// 	}
	
	texCoord = texCoordX[0]; 
	t_tile = t_tileX[0]; 
	te_normal = te_normalX[0]; 
// 	zoneColor = zc;
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	texCoord = texCoordX[1]; 
	t_tile = t_tileX[1]; 
	te_normal = te_normalX[1]; 
// 	zoneColor = zc;
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();
	
	texCoord = texCoordX[2]; 
	t_tile = t_tileX[2]; 
	te_normal = te_normalX[2]; 
// 	zoneColor = zc;
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();
	
	EndPrimitive();
}