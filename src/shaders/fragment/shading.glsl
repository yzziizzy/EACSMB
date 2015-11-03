#version 400


uniform sampler2D sDiffuse;
uniform sampler2D sNormals;
uniform sampler2D sDepth;
uniform sampler2D sSelection;

uniform int debugMode;

uniform vec3 sunNormal;

out vec4 FragColor;

void main() {
	vec2 tex = gl_FragCoord.xy / 600;
	
	if(debugMode == 0) {
		// normal rendering
		vec3 d = texture(sDiffuse, tex).rgb;
		vec3 amb = d * vec3(.2,.2,.2) * 2.9;
		vec3 sun = d * vec3(1.1, 1.0, .9) * 1.2* dot(sunNormal, texture(sNormals, tex).xyz);
		FragColor = vec4(amb + sun,  1.0);

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
	else if(debugMode == 4) {
		// diffuse
		FragColor = vec4(texture(sSelection, tex).rgb / 1024,  1.0);
	}

//	FragColor = vec4(texture(sNormals, tex).rgb,  1.0);
} 