#ifndef __common_gl_h__
#define __common_gl_h__


#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "common_math.h"

#include "utilities.h"
#include "shader.h"
#include "texture.h"
#include "objloader.h"



// GLEW lacks this for some reason
typedef  struct {
	GLuint  count;
	GLuint  instanceCount;
	GLuint  first;
	GLuint  baseInstance;
} DrawArraysIndirectCommand;




#endif // __common_gl_h__
