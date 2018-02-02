
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
uniform sampler2D sTexture;

in vec2 tc;
in float alpha;

out vec4 FragColor;

void main(void) {
	
	
	vec4 tex = texture(sTexture, tc.xy).rgba;
	
	//if(tex.a < 0.5) discard;
	
	
	FragColor = vec4(tex.rgba);
}

