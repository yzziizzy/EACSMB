

#shader VERTEX


#version 400


layout (location = 0) in vec3 pos;


void main() {
	gl_Position = vec4(pos, 1.0);
}






#shader FRAGMENT


#version 400

#include "include/lighting_pbr.glsl"

uniform sampler2D sDiffuse;
uniform sampler2D sNormals;
uniform sampler2D sMaterial;
uniform sampler2D sDepth;
uniform sampler2D sLighting;

uniform sampler2D sShadow;

uniform int debugMode;
uniform vec2 clipPlanes;
uniform vec2 shadowClipPlanes;

uniform vec3 sunNormal; // in world coordinatess

// forward matrices
uniform mat4 mWorldView;
uniform mat4 mViewProj;

// inverse matrices
uniform mat4 mViewWorld;
uniform mat4 mProjView;

uniform mat4 mWorldLight;

uniform vec2 resolution;


float linearizeDepth(float depth, vec2 clip) {
	return (
		(2.0 * clip.x * clip.y) / 
		(clip.y + clip.x - (depth * 2.0 - 1.0) * (clip.y - clip.x))
	) / clip.y;
}

out vec4 FragColor;









void main() {
	vec2 tex = gl_FragCoord.xy / resolution.xy;
	vec4 raw_normal = texture(sNormals, tex);
	vec4 raw_diffuse = texture(sDiffuse, tex);
	vec3 normal = (raw_normal.xyz * 2.0) - 1.0;
	vec3 diffuseColor = raw_diffuse.rgb;
	float metallic = texture(sMaterial, tex).r;
	float roughness = texture(sMaterial, tex).g;
	
	if(raw_normal.xyz != vec3(0,0,0)) normal = normalize(normal);

	
	if(debugMode == 0) {
		// normal rendering

		// reconstruct world coordinates
		vec2 screenCoord = gl_FragCoord.xy / resolution.xy;
		
		float depth = texture(sDepth, screenCoord).r;
		if (depth > 0.99999) {
			discard; // probably shouldn't be here
		}
		
		float ndc_depth = depth * 2.0 - 1.0;
		
// 		mat4 invVP = inverse(mViewProj * mWorldView);
		mat4 invVP = inverse(mViewProj * mWorldView);
// 		
		vec4 tmppos = invVP * vec4(screenCoord * 2.0 - 1.0, ndc_depth, 1.0);
		vec3 pos = tmppos.xyz / tmppos.w;
		// --------------------------------
		// pos is in world coordinates now
		
		vec3 viewdir_w = normalize((inverse(mViewProj * mWorldView) * vec4(0,0,1,1)).xyz);
		vec3 viewpos_w = (inverse(mWorldView) * vec4(0,0,0,1)).xyz;
	
 		viewdir_w = normalize(viewpos_w - pos);
//  		viewdir_w = normalize(pos - viewpos_w);
//  		FragColor = vec4(viewdir_w * .5 + .5, 1.0);
//  		return;
		
		// input setup 
		vec4 light_dir = inverse(mWorldLight) * vec4(0,0,1,1);
		light_dir = vec4(normalize(light_dir.xyz / light_dir.w), 1);
		
		light_dir = vec4(sunNormal.xyz, 1.0);
		
		
		
		// TODO: gather inputs
// 		vec3 dielectricSpecular = vec3(0.45, 0.45, 0.45);
// 		vec3 dielectricSpecular = vec3(1.0, 1.0, 1.0);
		vec3 dielectricSpecular = vec3(0.1, 0.1, 0.1);
//  		metallic = 0.408;
		vec3 baseColor = diffuseColor;
//  		roughness = 0.51;
		
		// TODO: gather vectors
		vec3 l = light_dir.xyz;//vec3(0,1,0); // light direction
		vec3 h = normalize(viewdir_w + l);//vec3(0,1,0); // half-vector
		
		
		
		// ---- shadow stuff ---------
		vec4 l_pos4 = mWorldLight * vec4(pos, 1.0);
		vec3 l_pos = vec3(l_pos4.xyz / l_pos4.w);
		//l_pos /= 1024;
		l_pos = l_pos * 0.5 + 0.5;
	//	float l_closest = texture(sShadow, l_pos.xy).r;
		float l_current = l_pos.z;
// 		vec4 light_dir = inverse(mWorldLight) * vec4(0,0,1,1);
		light_dir = vec4(normalize(light_dir.xyz / light_dir.w), 1);
	
		
	//	float bias = 0.005;
		// very broken
		float bias = max(0.03 * (1.0 - dot(normal, light_dir.xyz)), 0.0005);  
		float shadow_factor = 0.0;
		
		// PCF
		vec2 texelSz = 1.0 / textureSize(sShadow, 0);
		int sz = 1;
		for(int x = -sz; x <= sz; x++) {
			for(int y = -sz; y <= sz; y++) {
				float sf = texture(sShadow, l_pos.xy + (vec2(x, y) * texelSz)).r;
				shadow_factor += l_current - bias > sf ? 1 : 0;
			}
		}
		
		shadow_factor = smoothstep(0.0, 1.0, shadow_factor / ((sz + sz + 1)*(sz + sz + 1)));
		
		if(l_pos.x > 1 || l_pos.y > 1 || l_pos.x < 0 || l_pos.y < 0) {
			shadow_factor = 0;
		}
		
		// ^^^ shadow stuff ^^^^^^^^^^^
		
		
		vec3 sun_shad = f_Schlick_Smith_GGX(
			normal, h, l, viewdir_w, 
			baseColor, 
			metallic, roughness) * (1.0 - shadow_factor);
			
		vec3 colorOut = clamp(clamp(sun_shad, 0.0, 1.0) + texture(sLighting, tex).rgb, 0.0, 1.0);
		
		// gamma correction
		FragColor = vec4(pow(colorOut, vec3(1.1)), 1.0);
	
		
		//FragColor = vec4(light_dir.xyz, 1.0);


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
		// metallic/roughness
		FragColor = vec4(metallic, roughness, 1.0,  1.0);
	}
	else if(debugMode == 4) { 
		// ambient occlusion
		FragColor = vec4(vec3(texture(sMaterial, tex).b),  1.0);
	}
	else if(debugMode == 5) {
		// depth
		
		// bring it back to linear
		float nd = (
				(2.0 * clipPlanes.x * clipPlanes.y) / 
				(clipPlanes.y + clipPlanes.x - (texture(sDepth, tex).r * 2.0 - 1.0) * (clipPlanes.y - clipPlanes.x))
			) / clipPlanes.y;
		
		FragColor = vec4(vec3(nd),  1.0);
	}
	else if(debugMode == 6) {
		// lighting buffer
		FragColor = vec4(texture(sLighting, tex).rgb,  1.0);
	}
	else if(debugMode == 7) {
		// shadow depth
		
		// bring it back to linear
		float nd = (
				(2.0 * shadowClipPlanes.x * shadowClipPlanes.y) / 
				(shadowClipPlanes.y + shadowClipPlanes.x - (texture(sShadow, tex).r * 2.0 - 1.0) * (shadowClipPlanes.y - shadowClipPlanes.x))
			) / shadowClipPlanes.y;
		
		FragColor = vec4(vec3(nd),  1.0);
	//	FragColor = vec4(texture(sShadow, tex).rrr,  1.0);
	}
// 	else if(debugMode == 6) {
		// experimental PBR
		
		// normal rendering
		
// 	}
	
//	FragColor = vec4(texture(sNormals, tex).rgb,  1.0);
}
