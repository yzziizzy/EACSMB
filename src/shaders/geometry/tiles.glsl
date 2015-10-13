 
#version 430 core

// geometry shader 

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

in VS_OUT
{
    vec4 color;
} gs_in[4];

out GS_OUT
{
    flat vec4 color[4];
} gs_out;

void main(void)
{
	mat4 MV = gxl3d_ModelViewMatrix;
	
	vec3 right = vec3(MV[0][0],
	                  MV[1][0],
	                  MV[2][0]);
	
	vec3 up = vec3(MV[0][1],
	               MV[1][1],
	               MV[2][1]);
	vec3 P = gl_in[0].gl_Position.xyz;
	
	mat4 VP = gxl3d_ViewProjectionMatrix;
	
	vec3 va = P - (right + up) * size;
	gl_Position = VP * vec4(va, 1.0);
	Vertex_UV = vec2(0.0, 0.0);
	Vertex_Color = vertex[0].color;
	EmitVertex();
	
	vec3 vb = P - (right - up) * size;
	gl_Position = VP * vec4(vb, 1.0);
	Vertex_UV = vec2(0.0, 1.0);
	Vertex_Color = vertex[0].color;
	EmitVertex();
	
	vec3 vd = P + (right - up) * size;
	gl_Position = VP * vec4(vd, 1.0);
	Vertex_UV = vec2(1.0, 0.0);
	Vertex_Color = vertex[0].color;
	EmitVertex();
	
	vec3 vc = P + (right + up) * size;
	gl_Position = VP * vec4(vc, 1.0);
	Vertex_UV = vec2(1.0, 1.0);
	Vertex_Color = vertex[0].color;
	EmitVertex();
	
	EndPrimitive(); 
}