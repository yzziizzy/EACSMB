





typedef enum {
	RCT_Invalid = 0,
	RCT_DrawArraysInstanced
	
	
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




typedef struct DrawCommand {
	GLuint prog;
	
	GLuint vao;
	GLuint vbo1;
	GLuint vbo2;
	
	char patchVertices;
	char depthEnable;
	char depthMask;
	char blendEnable;
	GLenum blendFuncSrc;
	GLenum blendFuncDst;
	
	union {
		RC_DrawArraysInstanced DrawArraysInstanced;
		
	}
	
	
} DrawCommand;

struct UniformCommand;


typedef struct RenderCommand {
	RenderCommandType type;
	RenderCommandKey key;
	union {
		UniformCommand uniform;
		DrawCommand draw;
	}
} RenderCommand;


typedef struct RenderCommandQueue {
	int alloc_len;
	int next_index;
	RenderCommand* cmds;
} RenderCommandQueue


void render_RenderCommandQueue(RenderCommand* rc, int length); 
RenderCommandQueue* render_AllocCommandQueue(int size); 
RenderCommand* render_GetCommandSlot(RenderCommandQueue* rcq);









