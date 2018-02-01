#ifndef __EACSMB__vegetation_h__
#define __EACSMB__vegetation_h__


#include "common_gl.h"
#include "common_math.h"

#include "ds.h"






enum UNIFORM_TYPE {
	UT_INT,
	UT_UINT,
	UT_FLOAT,
	UT_DOUBLE,
	UT_MATRIX_2x2,
	UT_MATRIX_2x3,
	UT_MATRIX_2x4,
	UT_MATRIX_3x2,
	UT_MATRIX_3x3,
	UT_MATRIX_3x4,
	UT_MATRIX_4x2,
	UT_MATRIX_4x3,
	UT_MATRIX_4x4,
};



struct uniform_binding {
	char* name;
	GLuint location;
	enum UNIFORM_TYPE type;
	int size;
	int count;
	char transpose;
	void* data;
};


struct vert_attrib_info {
	GLenum type;
	int index;
	int count;
	GLbool normalized;
	int divisor;
	
	int size;
};


struct vertex_buffer_info {
	char* name;
	GLuint id;
	VEC(vert_attrib_info) attribs;
	
	int stride; // calculated
};

// TODO: vbo range bindings

typedef struct Drawable {
	
	ShaderProgram* prog; // ultimately, a drawable is most tightly coupled to the shader
	
	GLuint vao;
	
	VEC(struct vertex_buffer_info) vbos;
	VEC(struct uniform_binding) uniforms;
	
	GLenum primType;
	int firstIndex;
	int drawCnt;
	int instCnt;
	
	GLenum indexType;
	GLuint indexBuffer_id;
};





typedef struct Vegetation {
	
	
	
	Drawable* d;
	
} Vegetation;






#endif // __EACSMB__vegetation_h__
