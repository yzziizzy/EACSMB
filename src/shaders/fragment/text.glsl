#version 400


// fragment shader
uniform sampler2D fontTex;

in vec2 texCoord;



void main(void) {
// 	gl_FragColor = vec4(texCoord.x, texCoord.y, .5, 0);// texture2D(fontTex, texCoord).rrrr;
	gl_FragColor = texture2D(fontTex, texCoord).rrrr;
// 	out_Color = vec4(1.0, 0, .5, 0);
}