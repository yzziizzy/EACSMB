#version 430 core


layout (vertices = 4) out;


// in ivec2 tex_in[];
// in ivec2 tess_in[];


void main() {

    
//     gl_TessLevelOuter[0] = tess_in[gl_InvocationID].x; // x
//     gl_TessLevelOuter[1] = tess_in[gl_InvocationID].y; // y
//     gl_TessLevelOuter[2] = tess_in[gl_InvocationID].x; // x 
//     gl_TessLevelOuter[3] = tess_in[gl_InvocationID].y; // y
//     
//     gl_TessLevelInner[0] = tess_in[gl_InvocationID].x;
//     gl_TessLevelInner[1] = tess_in[gl_InvocationID].y;
//     
//     gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
//     
    
	gl_TessLevelOuter[0] = 64; 
	gl_TessLevelOuter[1] = 64; 
	gl_TessLevelOuter[2] = 64; 
	gl_TessLevelOuter[3] = 64; 

	gl_TessLevelInner[0] = 64;
	gl_TessLevelInner[1] = 64;

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
} 