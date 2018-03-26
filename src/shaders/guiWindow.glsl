
#shader VERTEX


#version 400 core

layout (location = 0) in vec2 pos_in;

uniform mat4 mProj;

uniform vec4 tlx_tly_w_h;
uniform vec4 z_alpha_borderWidth_fadeWidth;
uniform vec4 border_color;


out float alpha;
out vec2 tc;
out vec4 borderColor;
out float borderWidth;
out float fadeWidth;

void main() {

	vec2 pos = tlx_tly_w_h.xy;
	vec2 sz = tlx_tly_w_h.zw;
	
	pos += sz * pos_in;
	
	float z = z_alpha_borderWidth_fadeWidth.x;
	
	alpha = z_alpha_borderWidth_fadeWidth.y;
	borderWidth = z_alpha_borderWidth_fadeWidth.z;
	fadeWidth = z_alpha_borderWidth_fadeWidth.w;
	tc = pos_in.xy;
	borderColor = border_color;
	gl_Position = mProj * vec4(pos, z, 1.0); //(mProj) * vec4(pos, .5, 1.0);
}




#shader FRAGMENT

#version 400


// fragment shader

uniform vec3 color;



in vec2 tc;
in float alpha;
in vec4 borderColor;
in float borderWidth;
in float fadeWidth;

out vec4 FragColor;

void main(void) {
	
	// fade the edges
	float ei1 = smoothstep(0.0, borderWidth, tc.x);
	float ei2 = 1.0 - smoothstep(1.0 - borderWidth, 1.0, tc.x);
	float ei3 = smoothstep(0.0, borderWidth, tc.y);
	float ei4 = 1.0 - smoothstep(1.0 - borderWidth, 1.0, tc.y);
	
	vec4 edgeFactor = vec4(min(min(ei1, ei2), min(ei3, ei4)), 0,0,1).rrra;
	
	float bi1 = smoothstep(0.0, fadeWidth, tc.x);
	float bi2 = 1.0 - smoothstep(1.0 -fadeWidth, 1.0, tc.x);
	float bi3 = smoothstep(0.0, fadeWidth, tc.y);
	float bi4 = 1.0 - smoothstep(1.0 - fadeWidth, 1.0, tc.y);
	
	vec4 borderFactor = vec4(min(min(bi1, bi2), min(bi3, bi4)), 0,0,1).rrra;
	vec3 bc = borderColor.rgb; 
	float ba = borderColor.a;
	
	FragColor = vec4(mix(bc * ba, color, borderFactor.r), alpha * edgeFactor);
}

