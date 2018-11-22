
#shader VERTEX


#version 430 core

layout(std140) uniform; 

// constant attributes
layout (location = 0) in vec4 start_pos_offset_in;
layout (location = 1) in vec4 start_vel_spawndelay_in;
layout (location = 2) in vec4 start_acc_lifetime_in;
layout (location = 3) in vec4 size_spin_growth_random_in;
layout (location = 4) in vec4 fade_in_out;

// instanced attributes
layout (location = 5) in vec4 pos_scale_in;
layout (location = 6) in vec4 starttime_lifespan_in;



uniform mat4 mWorldView;
uniform mat4 mViewProj;


float timeSeconds;
float timeFractional;




flat out int vs_VertexID;

out Vertex {
  float size;
  float spin;
  float opacity;
  vec3 pos;
  
} vertex;

//                           |-----lifetime----------------------------|
// --offset--[--spawn-delay--|--fade-in--|--<calculated>--|--fade-out--]
//           [--------t_loop-------------------------------------------]      
//                           [------t----------------------------------]
void main() {
	vec3 instancePos = pos_scale_in.xyz;
	
	vec3 spritePos = start_pos_offset_in.xyz;
	
	float start_offset = start_pos_offset_in.w;
	float lifetime = start_acc_lifetime_in.w;
	float spawn_delay = start_vel_spawndelay_in.w;
	
	float time = mod(timeSeconds, lifetime + spawn_delay) + timeFractional;
	float t_loop = mod(time + start_offset, lifetime + spawn_delay);
	
	float t = t_loop - spawn_delay;
	
	if(t < 0) {
		vertex.opacity = 0;
		vertex.size = 0;
		return;
	}
	
	
	vec3 sim = t*t*start_acc_lifetime_in.xyz + t*start_vel_spawndelay_in.xyz;
	
	gl_Position = mWorldView * vec4(spritePos + instancePos + sim, 1.0);
	vs_VertexID = gl_VertexID;
	
	vertex.size = size_spin_growth_random_in.x + size_spin_growth_random_in.z * t;
	vertex.spin = mod(size_spin_growth_random_in.y * t, 2*3.1415926536);
	
	if(t < fade_in_out.x) {
		vertex.opacity = mix(0.0, 1.0, t / fade_in_out.x);
	}
	else if(lifetime - t < fade_in_out.y) {
		vertex.opacity = mix(0.0, 1.0, (lifetime - t) / fade_in_out.y);
	}
	else {
		vertex.opacity = 1.0;
	}
}


#shader GEOMETRY

#version 430 core

// geometry shader 

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;


uniform mat4 mWorldView;
uniform mat4 mViewProj;

flat in int vs_VertexID[];

out vec3 gs_color;
out vec3 gs_tex;
out float gs_opacity;

in Vertex {
	float size;
	float spin;
	float opacity;
	vec3 pos;
} vertex[];

void main() {
	if(vertex[0].opacity == 0.0) return;

	//mat4 mWorldProj = mViewProj * mWorldView;

	float size = vertex[0].size;
	float spin = vertex[0].spin;

	vec4 center = gl_in[0].gl_Position;
	
	mat2 rot = mat2(cos(spin), sin(spin), -sin(spin), cos(spin)); 
	
	float right = 0.5 * size;
	float up = 0.5 * size;

	gs_color = vec3(vs_VertexID[0] * .001, 1,0);
	gs_tex = vec3(0,0,0);
	gs_opacity = vertex[0].opacity;
	gl_Position = mViewProj * vec4(center.xy + vec2(-right, -up) * rot, center.zw);
	EmitVertex();

	gs_color = vec3(vs_VertexID[0] * .001, 1,1);
	gs_tex = vec3(0,1,0);
	gs_opacity = vertex[0].opacity;
	gl_Position = mViewProj * vec4(center.xy + vec2(-right, up) * rot, center.zw);
	EmitVertex();

	gs_color = vec3(vs_VertexID[0] * .001, 0,1);
	gs_tex = vec3(1,0,0);
	gs_opacity = vertex[0].opacity;
	gl_Position = mViewProj * vec4(center.xy + vec2(right, -up) * rot, center.zw);
	EmitVertex();

	gs_color = vec3(vs_VertexID[0] * .001, 0,0);
	gs_tex = vec3(1,1,0);
	gs_opacity = vertex[0].opacity;
	gl_Position = mViewProj * vec4(center.xy + vec2(right, up) * rot, center.zw);
	EmitVertex();

	EndPrimitive(); 
}



#shader FRAGMENT

#version 430 core



uniform mat4 mWorldView;
uniform mat4 mViewProj;

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;

uniform sampler2D textures;

in vec3 gs_color;
in vec3 gs_tex;
in float gs_opacity;

void main(void) {
    
	out_Color = texture(textures, gs_tex.xy) * vec4(1.0, 1.0, 1.0, gs_opacity); //vs_norm;
// 	out_Color = vec4(1.0, 1.0, 1.0, gs_opacity); //vs_norm;
	out_Color = vec4(1.0, 0.0, 0.0, 1.0); //vs_norm;
	out_Normal = vec4(0,0,0,0);
	
}

