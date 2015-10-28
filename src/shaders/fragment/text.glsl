#version 400


// fragment shader
uniform sampler2D fontTex;

in vec2 texCoord;
in vec4 color;



void main(void) {
//  	gl_FragColor = vec4(texCoord.x, texCoord.y, .5, 0);// texture2D(fontTex, texCoord).rrrr;
	
	float alpha = texture2D(fontTex, texCoord).r;
	
	if(alpha < .1) {
		discard;
	}
	else {
		gl_FragColor = alpha * vec4(color.a, color.b, color.g, color.r); // probably a better way to do this. it's late, meh
		
	}
// 	out_Color = vec4(1.0, 0, .5, 0);
}