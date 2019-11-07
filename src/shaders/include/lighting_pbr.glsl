




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

vec3 F_Schlick(vec3 dielectricSpecular, float metallic, float d_nv) {
	vec3 F_0 = dielectricSpecular;
	return F_0 + (1 - F_0) * pow(1 - d_nv, 5);
}


vec3 F_Blinn(vec3 baseColor, float d_nh) {
	return baseColor + (1 - baseColor) * pow(1 - (d_nh), 5);
}

vec3 Diff(vec3 dielectricSpecular, vec3 baseColor, float metallic, float d_nv) {
	vec3 black = vec3(0,0,0);
	return (1 - F_Schlick(dielectricSpecular, metallic, d_nv)) *
		(mix(baseColor * (1 - dielectricSpecular), black, metallic) / PI);
}

vec3 Diff_Lambert(float d_nl) {
	return vec3(max(0.0, d_nl));
}

vec3 f_Schlick_Smith_GGX(
	vec3 n, vec3 h, vec3 l, vec3 v, 
	vec3 baseColor,
	float metallic, float roughness
) {
// 	roughness = .51;
	
	float a = roughness * roughness;
	float a2 = a * a;
	
	// clamped dot products
	float d_nh = clamp(dot(n, h), 0.0, 1.0);
	float d_nl = clamp(dot(n, l), 0.0, 1.0); // surface normal dotted to light dir
	float d_lh = clamp(dot(l, h), 0.0, 1.0); // light dir and halfdir (light + view)
	float d_nv = clamp(dot(n, v), 0.0001, 1.0); // surface normal dotted to the view
	float d_vh = clamp(dot(h, v), 0.0, 1.0); 
 	//return vec3(d_nv,-d_nv, 0);

	
	// temp debug
	vec3 black = vec3(0,0,0);
	
/*
	D = microfacet distribution factor (roughness)
	F = fresnel reflection coefficient (shininess)
	G = geometric attenuation factor (self-shadowing)
*/

	
	vec3 dielectricSpecular = vec3(.91,.91,.91);
	
	// v.h is used when in a CT equation. v.l is used without it.
	vec3 schl = F_Schlick(dielectricSpecular, metallic, d_vh);
	float dg = D_GGX(d_nh, a); 
	float gs = G_Smith(d_nl, d_nv, a2); 
	
	vec3 top = schl * dg * gs;
	
	vec3 CookTorrence = top / clamp(4 * d_nl * d_nv, 0.00001, 100000.0); 
	
	float k = .2;
	
	vec3 diffuseTerm = baseColor * Diff_Lambert(d_nl);
	vec3 specularTerm = baseColor * (k + CookTorrence * (1.0 - k));
	
	
	float Ks = metallic;
	float Kd = 1.0 - Ks;
	
	return Kd * diffuseTerm + Ks * specularTerm;
}



