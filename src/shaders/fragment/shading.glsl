#version 400


uniform sampler2D sDiffuse;
uniform sampler2D sNormals;


out vec4 FragColor;

void main() {
	vec2 tex = gl_FragCoord.xy / 600;
	
	FragColor = vec4(texture(sDiffuse, tex).rgb, 1.0);
} 