#version 400


uniform sampler2D sDiffuse;
uniform sampler2D sNormals;
uniform sampler2D sDepth;

uniform int debugMode;

out vec4 FragColor;

void main() {
	vec2 tex = gl_FragCoord.xy / 600;
	
	if(debugMode == 0) {
		// normal rendering
		FragColor = vec4(texture(sDiffuse, tex).rgb,  1.0);

	}
	else if(debugMode == 1) {
		// diffuse
		FragColor = vec4(texture(sDiffuse, tex).rgb,  1.0);
	}
	else if(debugMode == 2) {
		// diffuse
		FragColor = vec4(texture(sNormals, tex).rgb,  1.0);
	}
	else if(debugMode == 3) {
		// diffuse
		FragColor = vec4(texture(sDepth, tex).rrr,  1.0);
	}

//	FragColor = vec4(texture(sNormals, tex).rgb,  1.0);
} 