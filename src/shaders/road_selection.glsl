
#shader VERTEX


#version 430 core

layout (location = 0) in vec4 pos_tex_in;
layout (location = 1) in vec4 cp02_in;
layout (location = 2) in vec2 cp1_in;


uniform sampler2DArray sHeightMap;
uniform sampler2D sOffsetLookup;

uniform mat4 mModel;
uniform mat4 mView;
uniform mat4 mProj;

// out vec4 vs_pos;
out vec4 vs_norm;
out vec2 vs_tex;
flat out vec2 vs_cp0;
flat out vec2 vs_cp1;
flat out vec2 vs_cp2;
out float vs_t;

flat out int ps_InstanceID;

float quad_bezier(float a, float b, float c, float t, float omt) {
	return (omt * omt * a) + (2 * omt * t * b) + (t * t * c);
}

float quad_bezier_dt(float a, float b, float c, float t, float omt) {
	return (2 * omt * (b - a)) + (2 * t * (c - b));
}

void main() {
	// y is the curve parameter
	float t = vs_t = pos_tex_in.y; 
	float omt = 1 - t;
	
	// x is 1 for the top corner, 2 for the bottom corner
	// the sign of x indicates which side this vertex is for
	float hwidth = sign(pos_tex_in.x);
	float voffset = abs(pos_tex_in.x) > 1.5 ? -1.0 : 0.0; 
	
	const vec2 cp0 = cp02_in.xy;
	const vec2 cp1 = cp1_in;
	const vec2 cp2 = cp02_in.zw;
	
	// this is the actual vertex location
	vec4 spline = vec4(
		quad_bezier(cp0.x, cp1.x, cp2.x, t, omt),
		quad_bezier(cp0.y, cp1.y, cp2.y, t, omt),
		0, 1);
		
	// tangent vector of the curve
	vec2 sdt = normalize(vec2(
		quad_bezier_dt(cp0.x, cp1.x, cp2.x, t, omt),
		quad_bezier_dt(cp0.y, cp1.y, cp2.y, t, omt)));
	
	// normal vector from the curve
	sdt = sdt.yx;
	sdt.x *= -0.5;
	
	sdt.x *= hwidth * 2;
	sdt.y *= hwidth * 2;
	
	vec4 pos = (spline + vec4(sdt.xy, 0, 1));
	
	pos.z = texture(sHeightMap, vec3(pos.x/256, pos.y/256, 0) , 0).r + (voffset * 4) + 2;

	gl_Position = (mProj * mView * mModel) * pos;
	vs_norm =  normalize(vec4(1,1,1,0));
	vs_tex = pos_tex_in.zw;
	vs_cp0 = cp0;
	vs_cp1 = cp1;
	vs_cp2 = cp2;
	
	vs_InstanceID = gl_InstanceID;
}




#shader FRAGMENT

#version 400

// the boxes themselves are used in the selection pass for speed and ease of selection

// in vec4 vs_pos;
in vec4 vs_norm;
in vec2 vs_tex;
flat in vec2 vs_cp0;
flat in vec2 vs_cp1;
flat in vec2 vs_cp2;
in float vs_t;

flat in int ps_InstanceID;

// fragment shader
uniform vec4 color;
uniform vec2 screenSize;

uniform int blockID;

uniform sampler2D sRoadTex;


layout(location = 2) out ivec4 out_Selection;



void main(void) {

	// x and y are the spline id.
	out_Selection = ivec4(ps_InstanceID / 256, ps_InstanceID % 256, blockID, 2);
}

