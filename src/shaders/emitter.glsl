
#shader VERTEX


#version 430 core

// constant attributes
layout (location = 0) in vec4 start_pos_fn_in;
layout (location = 1) in vec4 start_vel_spawndelay_in;
layout (location = 2) in vec4 start_acc_lifetime_in;
layout (location = 3) in vec4 size_spin_growth_random_in;

// instanced attributes
layout (location = 4) in vec4 pos_scale_in;
layout (location = 5) in vec4 starttime_lifespan_in;



uniform mat4 mView;
uniform mat4 mProj;

uniform float timeSeconds;
uniform float timeFractional;


flat out int vs_VertexID;

out Vertex {
  float size;
  vec3 pos;
  
} vertex;



void main() {
	vec3 instancePos = pos_scale_in.xyz;
	
	vec3 spritePos = start_pos_fn_in.xyz * 10;
	
	float time = mod(timeSeconds, start_acc_lifetime_in.w) + timeFractional;
	float t = mod(time + start_vel_spawndelay_in.w, start_acc_lifetime_in.w);
	
	vec3 sim = t*t*start_acc_lifetime_in.xyz + t*start_vel_spawndelay_in.xyz;
	
	//	gl_Position = (mProj * mView * mModel) * vec4(pos_tex_in.xy, 0, 1);
	gl_Position = vec4(spritePos + instancePos + sim, 1.0);
	vs_VertexID = gl_VertexID;
	
	vertex.size = size_spin_growth_random_in.x + size_spin_growth_random_in.z * t;
}


#shader GEOMETRY

#version 430 core

// geometry shader 

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;


uniform mat4 mView;
uniform mat4 mProj;

flat in int vs_VertexID[];

out vec3 gs_color;

in Vertex {
	float size;
	vec3 pos;
} vertex[];

void main() {
	mat4 mViewProj = mProj * mView;

	float size = vertex[0].size;

	vec3 right = vec3(mView[0][0], mView[1][0], mView[2][0]);
	vec3 up = vec3(mView[0][1], mView[1][1], mView[2][1]);

	vec3 center = gl_in[0].gl_Position.xyz;
	
	gs_color = vec3(vs_VertexID[0] * .05, 1,0);
	gl_Position = mViewProj * vec4((center - (right + up) * size).xyz, 1.0);
	EmitVertex();

	gs_color = vec3(vs_VertexID[0] * .05, 1,1);
		gl_Position = mViewProj * vec4((center - (right - up) * size).xyz, 1.0);
	EmitVertex();

	gs_color = vec3(vs_VertexID[0] * .05, 0,1);
	gl_Position = mViewProj * vec4((center + (right - up) * size).xyz, 1.0);
	EmitVertex();

	gs_color = vec3(vs_VertexID[0] * .05, 0,0);
	gl_Position = mViewProj * vec4((center + (right + up) * size).xyz, 1.0);
	EmitVertex();

	EndPrimitive(); 
}



#shader FRAGMENT

#version 430 core



uniform mat4 mView;
uniform mat4 mProj;

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;

in vec3 gs_color;

void main(void) {
    
	out_Color = vec4(gs_color.xyz, 1); //vs_norm;
}

