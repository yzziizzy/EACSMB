

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







/*
from glTF 2.0 launch doc (which has numerous errors; shame, shame, Khronos)
https://www.khronos.org/assets/uploads/developers/library/2017-web3d/glTF-2.0-Launch_Jun17.pdf

extra (correct) reference:
http://www.trentreed.net/blog/physically-based-shading-and-image-based-lighting/


l is light direction
n is normal vector
h is half vector
v is view direction

f(l,v,h) = Diff(l,h) + ( (F(l,n) * G(l,v,n) * D(h)) / (4 * dot(n,l) * dot(n,v)) ) 

Diff(l,h) = (1 - F(v, h)) * (Cdiff / pi)

const dielectricSpecular = rgb(0.04, 0.04, 0.04)
const black = rgb(0, 0, 0)

Cdiff = lerp(baseColor.rgb * (1 - dielectricSpecular), black, metallic)

// Fresnel Function: (schlick)
F(v,h) = F_0 + dot((1 - F_0), (1 - dot(v,h))^5)

// F_0 is the specular reflectance at normal incidence
F_0 = lerp(dielectricSpecular, baseColor.rgb, metallic)

//G is the geometric occlusion derived from a normal distribution function like Smith’s function
G(l,v,n) = G_1(n,l) * G_1(n,v)

G_1(n,v) = (2 * dot(n,v)) / ( dot(n,v) + sqrt(a^2 + (1 - a^2) * dot(n,v)^2) )

a = roughness^2

// D is the normal distribution function like GGX that defines the statistical distribution of microfacets
D(h) = a^2 / ( pi * (dot(n,h)^2 * (a - 1) + 1)^2 )
*/

#define PI 3.1415926535897932384626433832795

float D_GGX(float d_nh, float a) {
	float q = d_nh * d_nh * ((a * a) - 1) + 1;
	return (a * a) / (PI * q * q);
}

float G_1_Smith(float d_nv, float a2) {
	return (2 * d_nv) / (d_nv + sqrt(a2 + (1 - a2) * d_nv * d_nv));
}

float G_Smith(float d_nl, float d_nv, float a2) {
	return G_1_Smith(d_nl, a2) * G_1_Smith(d_nv, a2);
}

vec3 F_Schlick(vec3 dielectricSpecular, vec3 baseColor, float metallic, float d_nv) {
	vec3 F_0 =dielectricSpecular;// mix(dielectricSpecular, baseColor, metallic);
	return F_0 + (1 - F_0) * pow(1 - d_nv, 5);
}


vec3 F_Blinn(vec3 baseColor, float d_nh) {
	return baseColor + (1 - baseColor) * pow(1 - (d_nh), 5);
}

vec3 Diff(vec3 dielectricSpecular, vec3 baseColor, float metallic, float d_nv) {
	vec3 black = vec3(0,0,0);
	return (1 - F_Schlick(dielectricSpecular, baseColor, metallic, d_nv)) *
		(mix(baseColor * (1 - dielectricSpecular), black, metallic) / PI);
}

vec3 Diff_Lambert(vec3 baseColor, float d_nl) {
	return vec3(max(0.0, d_nl));
}

vec3 f_Schlick_Smith_GGX(
	vec3 n, vec3 h, vec3 l, vec3 v, 
	vec3 dielectricSpecular, vec3 baseColor,
	float metallic, float roughness
) {
	roughness = .51;
	
	float a = roughness * roughness;
	float a2 = a * a;
	
	float d_nh = clamp(dot(n, h), 0.0, 1.0);
	float d_nl = clamp(dot(n, l), 0.0, 1.0);
	float d_lh = clamp(dot(l, h), 0.0, 1.0);
	float d_nv = clamp(dot(n, v), 0.0001, 1.0);
	float d_vh = clamp(dot(h, v), 0.0, 1.0);
 	//return vec3(d_nv,-d_nv, 0);

	
	// temp debug
	vec3 black = vec3(0,0,0);
	
// 	return Diff(dielectricSpecular, baseColor, metallic, d_vh);
// 	return F_Schlick(dielectricSpecular, baseColor, metallic, d_vh);
// 	return Diff_Lambert(baseColor, d_nl);
//  	return (mix(baseColor * (1 - dielectricSpecular), black, 0.1) / PI);
//  	return (mix(baseColor * (1 - dielectricSpecular), black, metallic) / PI);
	
/*
	D = microfacet distribution factor (roughness)
	F = fresnel reflection coefficient (shininess)
	G = geometric attenuation factor (self-shadowing)
*/

	metallic = 0.71;
	vec3 zzz;
	zzz = vec3(0.0091,0.0091,0.0091);
	
// 	zzz = D_GGX(d_nh, a);
	
	dielectricSpecular = vec3(.91,.91,.91);
	
	zzz = F_Schlick(dielectricSpecular, baseColor, metallic, d_vh);
	//return zzz;
	float dg = D_GGX(d_nh, a); 
	float gs = G_Smith(d_nl, d_nv, a2); 
//	zzz = F_Blinn(baseColor, d_nh);
	//return vec3(dg,dg,dg);
	//return vec3(gs,gs,gs);
	
	vec3 top = zzz * dg * gs;
	
	vec3 CookTorrence = top / (PI * d_nl * d_nv); 
// 	return vec3((4 * d_nl * d_nv),(4 * d_nl * d_nv),(4 * d_nl * d_nv));
// 	return vec3(d_nv,-d_nv, 0);
// 	return n *.5 + .5;
	
	float k = .2;
	
	vec3 diffuseTerm = baseColor * Diff_Lambert(baseColor, d_nl);
// 	vec3 specularTerm = baseColor * pow(d_nh, 16.0); // blinn-phong
	vec3 specularTerm = baseColor * (k + CookTorrence * (1.0 - k));
	
	
	
	float Ks = metallic;
	float Kd = 1.0 - Ks;
	
	return Kd * diffuseTerm + Ks * specularTerm;
	
	
	vec3 c = baseColor; 
	
	return clamp(
		Kd * (Diff_Lambert(baseColor, d_nl) * baseColor /** (c / PI)*/) + (Ks * zzz / (4 * d_nl * d_nv)), 0.0, 1.0);

	
	return Diff(dielectricSpecular, baseColor, metallic, d_vh) + (
		(
			F_Schlick(dielectricSpecular, baseColor, metallic, d_lh) * // BUG: last arg may be wrong
			G_Smith(d_nl, d_nv, a2) *
			D_GGX(d_nh, a)
		) / (4 * d_nl * d_nv)
	);
}







void main() {
	vec2 tex = gl_FragCoord.xy / resolution.xy;
	vec4 raw_normal = texture(sNormals, tex);
	vec4 raw_diffuse = texture(sDiffuse, tex);
	vec3 normal = (raw_normal.xyz * 2.0) - 1.0;
	if(raw_normal.xyz != vec3(0,0,0)) normal = normalize(normal);
	
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
		
		
		// ---- shadow stuff ---------
		vec4 l_pos4 = mWorldLight * vec4(pos, 1.0);
		vec3 l_pos = vec3(l_pos4.xyz / l_pos4.w);
		//l_pos /= 1024;
		l_pos = l_pos * 0.5 + 0.5;
	//	float l_closest = texture(sShadow, l_pos.xy).r;
		float l_current = l_pos.z;
		vec4 light_dir = inverse(mWorldLight) * vec4(0,0,1,1);
		light_dir = vec4(normalize(light_dir.xyz / light_dir.w), 1);
	
		
	//	float bias = 0.005;
		float bias = max(0.005 * (1.0 - dot(normal, light_dir.xyz)), 0.0005);  
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
		
		shadow_factor = smoothstep(0, 1, shadow_factor / ((sz + sz + 1)*(sz + sz + 1)));
		
		if(l_pos.x > 1 || l_pos.y > 1 || l_pos.x < 0 || l_pos.y < 0) {
			shadow_factor = 0;
		}
		
		// ^^^ shadow stuff ^^^^^^^^^^^
		
		
		float specPower = raw_normal.a * 128;
		float specIntensity = raw_diffuse.a;
		
		
		vec3 pos_vw = (vec4(pos, 1) * mWorldView).xyz;
		
		//vec3 pos_vw = (vec4(pos, 1) * mWorldView).xyz;
		
		
		
		// world space
		vec3 viewpos = (inverse(mViewProj * mWorldView) * vec4(0,0,0,1)).xyz;
// 		vec3 viewpos = (invVP * vec4(0,0,0,1)).xyz;
		
// 		vec3 viewpos_v = (inverse(mWorldView) * vec4(0,0,0,1)).xyz;
		
		//vec3 viewdir = normalize(viewpos - pos);
		// world space
		vec3 viewdir = -normalize((inverse(mViewProj * mWorldView) * vec4(0,0,-1,1)).xyz);
		
		//normal = (mWorldView * vec4(normal, 1)).xyz;
		
		// voodoo: specular is inverted from diffuse otherwise. something is wrong
		//normal *= -1;
		
		// world space
		vec3 sundir = sunNormal;
		
		vec3 sunColor;
		
		sunColor = vec3(1, .9, .8) * (1 - shadow_factor);
		
		
		vec3 posToCam = normalize(viewpos - pos); 
		
		//------------ specular --------------------
		
		vec3 halfVec = normalize(viewdir - sundir);
		
		float nDl = max(0, dot(normal, sundir)); 
		float nDh = max(0, dot(normal, -halfVec)); 
		float specular = pow(nDh, specPower);
		
		if(nDl == 0) {
			specular = 0;
		}
		//specular = .5;
		////////-----------------////////

		
		vec3 specColor = vec3(1,.9,.8);//normalize(vec3(1,1,1));
		// ^^^^^^^^^^^^ specular ^^^^^^^^^^^^^^^^^^^
		
		vec3 diffuseColor = raw_diffuse.rgb;
		vec3 ambient = vec3(0.16,0.18,0.2);
		
		vec3 light = texture(sLighting, tex).rgb;
		
		
		vec3 colorOut;
		if(length(normal) < 1.01) { // things with normals get directional lighting
			colorOut = vec3(
				(((nDl * sunColor * 1.1) + light + ambient) * diffuseColor)
				+ (specular * specColor * diffuseColor * specIntensity)
			);
		}
		else { // no directional lighting for things without normals
			colorOut = vec3(ambient + diffuseColor);
		}
		
		// gamma correction
		FragColor = vec4(pow(colorOut, vec3(1.1)), 1.0);
		
		
// 		FragColor = vec4((l_pos4.xyz / l_pos4.w), 1.0);
//  		FragColor = vec4(l_pos.xy, 0 , 1.0);
//  		FragColor = vec4(l_closest, l_closest / 100, l_closest / 10000, 1.0);
//  		FragColor = vec4(l_pos.z,l_pos.z / 100,l_pos.z /1000, 1.0);
// 		FragColor = vec4(shadow_factor, shadow_factor / 100, shadow_factor / 1000, 1.0);
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
	else if(debugMode == 6) {
		// shadow depth
		
		// bring it back to linear
		float nd = (
				(2.0 * shadowClipPlanes.x * shadowClipPlanes.y) / 
				(shadowClipPlanes.y + shadowClipPlanes.x - (texture(sShadow, tex).r * 2.0 - 1.0) * (shadowClipPlanes.y - shadowClipPlanes.x))
			) / shadowClipPlanes.y;
		
		FragColor = vec4(vec3(nd),  1.0);
	//	FragColor = vec4(texture(sShadow, tex).rrr,  1.0);
	}
	else if(debugMode == 7) {
		// experimental PBR
		
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
		float metallic = 0.408;
		vec3 baseColor = texture(sDiffuse, tex).rgb;//vec3(0.45, 0.45, 0.45);
		float roughness = 0.1; // sampled from tex
		
		// TODO: gather vectors
		vec3 l = light_dir.xyz;//vec3(0,1,0); // light direction
		vec3 h = normalize(viewdir_w + l);//vec3(0,1,0); // half-vector
		
		FragColor = vec4(f_Schlick_Smith_GGX(
			normal, h, l, viewdir_w, 
			dielectricSpecular, baseColor, 
			metallic, roughness), 1);
		
		//FragColor = vec4(light_dir.xyz, 1.0);
	}
		
//	FragColor = vec4(texture(sNormals, tex).rgb,  1.0);
}
