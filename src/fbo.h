#ifndef __EACSMB_FBO_H__
#define __EACSMB_FBO_H__




typedef struct FBOConfig {
	GLenum attachment;
	GLuint texture;
	
} FBOConfig;

typedef struct FBOTexConfig {
	GLenum internalType;
	GLenum format;
	GLenum size;
} FBOTexConfig;



typedef struct Framebuffer {
	GLuint fb;
	
} Framebuffer;



GLuint* initFBOTextures(int w, int h, FBOTexConfig* cfg);
void initFBO(Framebuffer* fb, FBOConfig* cfg);
void destroyFBO(Framebuffer* fb);

#endif // __EACSMB_FBO_H__