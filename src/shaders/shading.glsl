

#shader VERTEX


#version 400


layout (location = 0) in vec3 pos;


void main() {
	gl_Position = vec4(pos, 1.0);
}






#shader FRAGMENT


#version 400


uniform sampler2D sDiffuse;
uniform sampler2D sNormals;
uniform sampler2D sDepth;
uniform sampler2D sSelection;
uniform sampler2D sLighting;

uniform int debugMode;
uniform vec2 clipPlanes;

uniform vec3 sunNormal;

// forward matrices
uniform mat4 mWorldView;
uniform mat4 mViewProj;

// inverse matrices
uniform mat4 mViewWorld;
uniform mat4 mProjView;

uniform vec2 resolution;


out vec4 FragColor;

void main() {
	vec2 tex = gl_FragCoord.xy / resolution.xy;
	vec3 normal = (texture(sNormals, tex).xyz * 2.0) - 1.0;
	
	if(debugMode == 0) {
		// normal rendering
		// reconstruct world coordinates
		vec2 screenCoord = gl_FragCoord.xy / resolution.xy;
		
		float depth = texture(sDepth, screenCoord).r;
		if (depth > 0.99999) {
		//	discard; // probably shouldn't be here
		}
		
		float ndc_depth = depth * 2.0 - 1.0;
		
// 		mat4 invVP = inverse(mViewProj * mWorldView);
		mat4 invVP = inverse(mViewProj * mWorldView);
// 		
		vec4 tmppos = invVP * vec4(screenCoord * 2.0 - 1.0, ndc_depth, 1.0);
		vec3 pos = tmppos.xyz / tmppos.w;
		// --------------------------------
		// pos is in world coordinates now
		
		vec3 pos_vw = (vec4(pos, 1) * mWorldView).xyz;
		
		//vec3 pos_vw = (vec4(pos, 1) * mWorldView).xyz;
		
		
		
		// world space
		vec3 viewpos = (inverse(mViewProj * mWorldView) * vec4(0,0,0,1)).xyz;
// 		vec3 viewpos = (invVP * vec4(0,0,0,1)).xyz;
		
// 		vec3 viewpos_v = (inverse(mWorldView) * vec4(0,0,0,1)).xyz;
		
		//vec3 viewdir = normalize(viewpos - pos);
		// world space
		vec3 viewdir = normalize((inverse(mViewProj * mWorldView) * vec4(0,0,-1,1)).xyz);
		
		//normal = (mWorldView * vec4(normal, 1)).xyz;
		
		// voodoo: specular is inverted from diffuse otherwise. something is wrong
		//normal *= -1;
		
		// world space
		vec3 sundir = sunNormal;
		
		
		float lambertian = max(dot(sundir, normal), 0.0);
		
		vec3 sunRefl = reflect(-sundir, normal);
		vec3 posToCam = normalize(viewpos - pos); 
		
		float specAngle = max(dot(posToCam, sunRefl), 0.0);
		float specular = pow(specAngle, 32);

		vec3 diffuseColor = texture(sDiffuse, tex).rgb;
		vec3 specColor = vec3(1,1,1) * .5;//normalize(vec3(1,1,1));

		vec3 ambient = vec3(0.1,0.1,0.1);
		
		vec3 light = texture(sLighting, tex).rgb;
		
		vec3 colorOut;
		if(length(normal) < 1.01) { // things with normals get directional lighting
			colorOut = vec3(
				ambient  
				+ (((lambertian * .7) + light) * diffuseColor)
				+ (specular * specColor)
			);
		}
		else { // no directional lighting for things without normals
			colorOut = vec3(ambient + diffuseColor);
		}
		
		// gamma correction
		FragColor = vec4(pow(colorOut, vec3(1.1)), 1.0);
		
		
	//	FragColor = vec4(sundir * .5 + .5, 1.0);
	//	FragColor = vec4(lambertian * diffuseColor, 1.0);
//		FragColor = vec4(vec3(dot(normalize(-sundir), normalize(normal))), 1.0);
//		
	}
	else if(debugMode == 1) {
		// diffuse
		FragColor = vec4(texture(sDiffuse, tex).rgb,  1.0);
	}
	else if(debugMode == 2) {
		// normals
 		FragColor = vec4(abs(texture(sNormals, tex).rgb * 2 - 1),  1.0);
	//	FragColor = vec4(texture(sNormals, tex).rgb,  1.0);
	}
	else if(debugMode == 3) {
		// depth
		
		// bring it back to linear
		float nd = (
				(2.0 * clipPlanes.x * clipPlanes.y) / 
				(clipPlanes.y + clipPlanes.x - (texture(sDepth, tex).r * 2.0 - 1.0) * (clipPlanes.y - clipPlanes.x))
			) / clipPlanes.y;
		
		FragColor = vec4(vec3(nd),  1.0);
	}
	else if(debugMode == 4) {
		// selection buffer
		FragColor = vec4(texture(sSelection, tex).rgb,  1.0);
	}
	else if(debugMode == 5) {
		// lighting buffer
		FragColor = vec4(texture(sLighting, tex).rgb,  1.0);
	}

//	FragColor = vec4(texture(sNormals, tex).rgb,  1.0);
}
