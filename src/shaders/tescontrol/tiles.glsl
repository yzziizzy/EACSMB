#version 430 core


layout (vertices = 4) out;



void main() {

    
    gl_TessLevelOuter[0] = 32; 
    gl_TessLevelOuter[1] = 32; 
    gl_TessLevelOuter[2] = 32; 
    gl_TessLevelOuter[3] = 32; 
    
    gl_TessLevelInner[0] = 32;
    gl_TessLevelInner[1] = 32;
    
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
} 