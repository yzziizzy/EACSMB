
#shader VERTEX


#version 430 core

layout (location = 0) in vec4 pos_tex_in;
layout (location = 1) in vec4 cp02_in;
layout (location = 2) in vec2 cp1_in;



uniform mat4 mModel;
uniform mat4 mView;
uniform mat4 mProj;

// out vec4 vs_pos;
out vec4 vs_norm;
out vec2 vs_tex;



float quad_bezier(float a, float b, float c, float t, float omt) {
	return (omt * omt * a) + (2 * omt * t * b) + (t * t * c);
}

float quad_bezier_dt(float a, float b, float c, float t, float omt) {
	return (2 * omt * (b - a)) + (2 * t * (c - b));
}

void main() {
	// y is the curve parameter
	float t = pos_tex_in.y; 
	float omt = 1 - t;
	
	// x is the length to scale the normal vector;
	// the sign of x indicates which side this vertex is for
	float hwidth = pos_tex_in.x;
	
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
	sdt.x *= -1;
	
	sdt *= -hwidth;
	
	//spline.x += hwidth;

//	gl_Position = (mProj * mView * mModel) * vec4(pos_tex_in.xy, 0, 1);
	gl_Position = (mProj * mView * mModel) * (spline + vec4(sdt.xy, 0, 1));
	vs_norm =  normalize(vec4(1,1,1,0));
	vs_tex = pos_tex_in.zw;
}




#shader FRAGMENT

#version 400

// in vec4 vs_pos;
in vec4 vs_norm;
in vec2 vs_tex;

// fragment shader
uniform vec4 color;


layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;


void main(void) {
	
	out_Color = vec4(1,0, 0, 1); //vs_norm;
	out_Normal = vs_norm;
}

