



typedef enum {
	RCT_Invalid = 0,
	RCT_DrawArrays
	
	
} RenderCommandType;


typedef uint64_t RenderCommandKey;


typedef struct {
	void (*setUniforms)(GLuint, void*);
	void* uniformData;
	
	GLenum mode,
	GLint first,
	GLsizei count,
	GLsizei primcount,
} RC_DrawArraysInstanced;


typedef struct RenderCommand {
	RenderCommandType type;
	RenderCommandKey key;
	
	GLuint prog;
	
	GLuint vao;
	GLuint vbo1;
	GLuint vbo2;
	
	char depthEnable;
	char depthMask;
	char blendEnable;
	GLenum blendFuncSrc;
	GLenum blendFuncDst;
	
	union {
		RC_DrawArraysInstanced DrawArraysInstanced;
		
	}
		
	
	// state here
	
}



















