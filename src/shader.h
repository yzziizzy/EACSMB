#ifndef __EACSMB_SHADER_H__
#define __EACSMB_SHADER_H__

 


 
typedef struct {
	char* name;
	GLuint id;
	

} Shader;
 
typedef struct {
	Shader* tcs;
	Shader* ts;
	Shader* vs;
	Shader* gs;
	Shader* fs;
	GLuint id;
	
} ShaderProgram;
 
Shader* loadShader(char* path, GLenum type);
ShaderProgram* loadProgram(char* vname, char* fname, char* gname, char* tcname, char* tname);
 
ShaderProgram* loadCombinedProgram(char* path);


#endif // __EACSMB_SHADER_H__
