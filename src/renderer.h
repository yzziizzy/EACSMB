



typedef enum {
	RCT_Invalid = 0,
	RCT_DrawArrays
	
	
} RenderCommandType;


typedef uint64_t RenderCommandKey;


typedef struct {
	
	GLenum mode,
	GLint first,
	GLsizei count,
	GLsizei primcount,
} RC_DrawArraysInstanced;


typedef struct RenderCommand {
	RenderCommandType type;
	RenderCommandKey key;
	
	union {
		RC_DrawArraysInstanced DrawArraysInstanced
		
		
	}
	
	// state here
	
}



















