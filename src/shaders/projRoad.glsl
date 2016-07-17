
#shader VERTEX


#version 430 core

layout (location = 0) in vec4 pos_tex_in;
layout (location = 1) in vec4 cp02_in;
layout (location = 2) in vec2 cp1_in;

// this texture is the active depth buffer, but depth writes are disabled for the
// decal pass. 
uniform sampler2D sDepth;

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
	//pos.z = (voffset * 3) + 200;
	//spline.x += hwidth;

//	gl_Position = (mProj * mView * mModel) * vec4(pos_tex_in.xy, 0, 1);
	gl_Position = (mProj * mView * mModel) * pos;
	vs_norm =  normalize(vec4(1,1,1,0));
	vs_tex = pos_tex_in.zw;
	vs_cp0 = cp0;
	vs_cp1 = cp1;
	vs_cp2 = cp2;
}




#shader FRAGMENT

#version 400

// in vec4 vs_pos;
in vec4 vs_norm;
in vec2 vs_tex;
flat in vec2 vs_cp0;
flat in vec2 vs_cp1;
flat in vec2 vs_cp2;
in float vs_t;

// fragment shader
uniform vec4 color;
uniform vec2 screenSize;

uniform sampler2D sDepth;

uniform mat4 mModel;
uniform mat4 mView;
uniform mat4 mProj;

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;



float quad_bezier(float a, float b, float c, float t, float omt) {
	return (omt * omt * a) + (2 * omt * t * b) + (t * t * c);
}

float quad_bezier_dt(float a, float b, float c, float t, float omt) {
	return (2 * omt * (b - a)) + (2 * t * (c - b));
}

void main(void) {

	vec2 screenCoord = gl_FragCoord.xy / screenSize;
	
	float depth = texture(sDepth, screenCoord).r;
	if (depth > 0.99999) {
		discard; // stencil later?
	}
	
	float ndc_depth = depth * 2.0 - 1.0;
	
	vec4 tmppos = inverse(mProj * mView) * vec4(screenCoord * 2.0 - 1.0, ndc_depth, 1.0);
    vec3 pos = tmppos.xyz / tmppos.w;
    
    // pos is in model-space, which is what our control points are in
    // TODO: block instance offset lookups
    
    vec3 npos = pos / 256;
    
    
    float t = vs_t;
    float omt = 1 - vs_t;
    
    // this is the actual vertex location
	vec4 spline = vec4(
		quad_bezier(vs_cp0.x, vs_cp1.x, vs_cp2.x, t, omt),
		quad_bezier(vs_cp0.y, vs_cp1.y, vs_cp2.y, t, omt),
		0, 1);
		
	// tangent vector of the curve
	vec2 sdt = normalize(vec2(
		quad_bezier_dt(vs_cp0.x, vs_cp1.x, vs_cp2.x, t, omt),
		quad_bezier_dt(vs_cp0.y, vs_cp1.y, vs_cp2.y, t, omt)));
	
	// normal vector from the curve
	sdt = sdt.yx;
	sdt.x *= -0.5;
	
	vec2 norm = normalize(sdt); 
	
	float width = abs(dot(pos.xy - spline.xy, norm));
    if(width > 1) discard;
    
	out_Color = vec4(width, 0 ,0 , 1); //vs_norm;
// 	out_Color = vec4(npos.xy, 0, 1); //vs_norm;
	out_Normal = vs_norm;
}

