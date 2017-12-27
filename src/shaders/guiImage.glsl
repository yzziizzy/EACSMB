
#shader VERTEX


#version 400 core

layout (location = 0) in vec2 pos_in;

uniform mat4 mProj;

uniform vec4 tlx_tly_w_h;
uniform vec4 z_alpha_;


out float alpha;
out vec2 tc;

void main() {

	vec2 pos = tlx_tly_w_h.xy;
	vec2 sz = tlx_tly_w_h.zw;
	
	pos += sz * pos_in;
	
	float z = z_alpha_.x;
	
	alpha = z_alpha_.y;
	tc = pos_in.xy;
	gl_Position = mProj * vec4(pos, z, 1.0); //(mProj) * vec4(pos, .5, 1.0);
}




#shader FRAGMENT

#version 400


// fragment shader

uniform vec3 color;
uniform sampler2DArray sTexture;
uniform int texIndex; 

in vec2 tc;
in float alpha;

out vec4 FragColor;

void main(void) {
	
	// fade the edges
	float ei1 = smoothstep(0.0, 0.02, tc.x);
	float ei2 = 1.0 - smoothstep(0.98, 1.0, tc.x);
	float ei3 = smoothstep(0.0, 0.02, tc.y);
	float ei4 = 1.0 - smoothstep(0.98, 1.0, tc.y);
	
	vec4 edgeFactor = vec4(min(min(ei1, ei2), min(ei3, ei4)), 0,0,1).rrra;

	vec4 tex = texture(sTexture, vec3(tc.xy, texIndex)).rgba;
	
	FragColor = vec4(tex.rgb, alpha * edgeFactor);
//	FragColor = vec4(vec3(.1,.2,.8), alpha * edgeFactor);
}

