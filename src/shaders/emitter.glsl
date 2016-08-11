
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


uniform sampler2D sTextures;

uniform mat4 mView;
uniform mat4 mProj;

// out vec4 vs_pos;
out vec4 vs_norm;
out vec2 vs_tex;
flat out vec2 vs_cp0;
flat out vec2 vs_cp1;
flat out vec2 vs_cp2;
out float vs_t;


out Vertex {
  vec3 pos;
} vertex;



void main() {
	
	
	vec4 spritePos = vec4(start_pos_fn_in.xyz, 1.0);
	
	//	gl_Position = (mProj * mView * mModel) * vec4(pos_tex_in.xy, 0, 1);
	gl_Position = spritePos;
	
}


#shader GEOMETRY

#version 430 core

// geometry shader 

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;


uniform mat4 mView;
uniform mat4 mProj;

in Vertex {
  vec3 pos;
} vertex[];

void main() {

	float size = 7;

	vec3 right = vec3(mView[0][0], mView[1][0], mView[2][0]);
	vec3 up = vec3(mView[0][1], mView[1][1], mView[2][1]);

	vec3 center = gl_in[0].gl_Position.xyz;
	
	gl_Position = mProj * vec4((center + (right - up) * size).xyz, 1.0);
	EmitVertex();

	gl_Position = mProj * vec4((center + (right + up) * size).xyz, 1.0);
	EmitVertex();

	gl_Position = mProj * vec4((center - (right - up) * size).xyz, 1.0);
	EmitVertex();

	gl_Position = mProj * vec4((center - (right + up) * size).xyz, 1.0);
	EmitVertex();

	EndPrimitive(); 
}



#shader FRAGMENT

#version 430 core



uniform mat4 mModel;
uniform mat4 mView;
uniform mat4 mProj;

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;



void main(void) {
    
	out_Color = vec4(1,1,1, 1); //vs_norm;
}

