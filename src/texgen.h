#ifndef __EACSMB_texgen_h__
#define __EACSMB_texgen_h__



#include "common_math.h"
#include "common_gl.h"


#include "gui.h"
#include "texture.h"
#include "perlin.h"
#include "opensimplex.h"


typedef enum {
	TEXGEN_SOLID,
	TEXGEN_LERP,
	TEXGEN_SINEWAVE,
	TEXGEN_ROTATE,
} TexGenType;



struct TG_solid {
	Vector4 color;
};
struct TG_sinewave {
	float period;
};
struct TG_lerp {
	char* name_A;
	char* name_B;
	char* name_Out;
	int channel_A;
	int channel_B;
	int channel_Out;
	float t;
};


typedef struct TexGenOp {
	TexGenType type;
	int channel_out;
	union {
		
		
	};
} TexGenOp;


typedef struct TexGen {
	
	
	
	
	Texture* output;
	
} TexGen;







typedef struct GUITexBuilderControl {
	GUIHeader header;
	
	GUISimpleWindow* bg;
	GUIImage* im;
	
	TexGen* tg;
	
	InputEventHandler* inputHandlers;
	
	
} GUITexBuilderControl;






GUITexBuilderControl* guiTexBuilderControlNew(Vector2 pos, Vector2 size, int zIndex);










#endif // __EACSMB_texgen_h__
