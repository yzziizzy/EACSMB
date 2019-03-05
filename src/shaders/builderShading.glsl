
#extension GL_ARB_bindless_texture : enable

#shader VERTEX


#version 400


layout (location = 0) in vec3 pos;


void main() {
	gl_Position = vec4(pos, 1.0);
}






#shader FRAGMENT


#version 400


layout(bindless_sampler) uniform sampler2D sDiffuse;
layout(bindless_sampler) uniform sampler2D sNormals;
layout(bindless_sampler) uniform sampler2D sDepth;
layout(bindless_sampler) uniform sampler2D sLighting;

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

layout(location = 0) out vec4 FragColor;

void main() {
	
	vec2 tex = gl_FragCoord.xy / resolution.xy;
	vec3 normal = texture(sNormals, tex).xyz * 2 - 1;
	
	
	if(1 == 1) {
		// normal rendering
		// reconstruct world coordinates
		vec2 screenCoord = gl_FragCoord.xy / resolution.xy;
		
		float depth = texture(sDepth, screenCoord).r;
		if (depth > 0.99999) {
		//	discard; // probably shouldn't be here
		}
		
		float ndc_depth = depth * 2.0 - 1.0;
		
		FragColor = vec4(vec3(ndc_depth), 1.0);
		//FragColor = vec4(normal, 1.0);
		return;
		
// 		mat4 invVP = inverse(mViewProj * mWorldView);
		mat4 invVP = mViewWorld * mProjView;
// 		
		vec4 tmppos = invVP * vec4(screenCoord * 2.0 - 1.0, ndc_depth, 1.0);
		vec3 pos = tmppos.xyz / tmppos.w;
		// pos is in world coordinates now
		
		vec3 viewpos = (inverse(mViewProj * mWorldView) * vec4(0,0,0,1)).xyz;
		
		vec3 viewdir = normalize(viewpos - pos);
		vec3 sundir = normalize(vec3(3,3,3));
		
		float lambertian = max(dot(sundir, normal), 0.0);
		
		vec3 halfDir = normalize(sundir + viewdir);
		float specAngle = max(dot(halfDir, normal), 0.0);
		float specular = pow(specAngle, 16);

		vec3 diffuseColor = texture(sDiffuse, tex).rgb;
		vec3 specColor = vec3(0,0,0);//normalize(vec3(1,1,1));

		vec3 ambient = vec3(0.1,0.1,0.1);
		if(length(normal) < 1.01) { // things with normals get directional lighting
			FragColor = vec4(ambient + lambertian * diffuseColor+ specular * specColor, 1.0);
		}
		else { // no directional lighting for things without normals
			FragColor = vec4(ambient + diffuseColor, 1.0);
		}
	}


//	FragColor = vec4(texture(sDiffuse, tex).rgba);
// 	FragColor = vec4(.10, 0.3, .20,  1.0);
}
