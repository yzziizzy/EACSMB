#ifndef __EACSMB_texgen_h__
#define __EACSMB_texgen_h__



#include "common_math.h"
#include "common_gl.h"


#include "gui.h"
#include "texture.h"
#include "perlin.h"
#include "opensimplex.h"


#define TEXGEN_TYPE_LIST \
	TEXGEN_TYPE_MEMBER(solid) \
	TEXGEN_TYPE_MEMBER(lerp) \
	TEXGEN_TYPE_MEMBER(sinewave) \
	TEXGEN_TYPE_MEMBER(perlin) \
	TEXGEN_TYPE_MEMBER(rotate) \
	TEXGEN_TYPE_MEMBER(chanmux)




typedef enum {
#define TEXGEN_TYPE_MEMBER(x) TEXGEN_TYPE_##x,
	TEXGEN_TYPE_LIST
#undef TEXGEN_TYPE_MEMBER
} TexGenType;



struct tg_reflect {
	char* name;
	ptrdiff_t member_offset;
	int type;
	union {
		double f;
		int64_t i;
		uint64_t u;
		char* s;
	} range_min, range_max, def_value;
};


#define TG_REFL_STRUCT_NAME solid
#define XLIST \
	X(Vector4, color, -1, -1, -1) 
#include "tg_reflect.h"
#undef XLIST


#define TG_REFL_STRUCT_NAME sinewave
#define XLIST \
	X(float, period, 0.00001, 9999999.0, 2.0) \
	X(float, phase, 0.0, 1.0, .25) 
#include "tg_reflect.h"
#undef XLIST

#define TG_REFL_STRUCT_NAME perlin
#define XLIST \
	X(float, persistence, 0.00001, 9999999.0, 2.0) \
	X(int, octaves, 0.0, 1.0, .25) 
#include "tg_reflect.h"
#undef XLIST

#define TG_REFL_STRUCT_NAME rotate
#define XLIST \
	X(int, flip, 0, 4, 0) 
#include "tg_reflect.h"
#undef XLIST

#define TG_REFL_STRUCT_NAME chanmux
#define XLIST \
	X(int, flip, 0, 4, 0) 
#include "tg_reflect.h"
#undef XLIST

//#define TG_REFL_STRUCT_NAME lerp
//#define XLIST \
	//X(int, flip, 0, 4, 0) 
//#include "tg_reflect.h"
//#undef XLIST


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
		struct TG_solid solid;
		struct TG_lerp lerp;
		struct TG_sinewave sinewave;
		struct TG_perlin perlin;
		struct TG_rotate rotate;
	};
} TexGenOp;


typedef struct TexGen {
	
	HashTable* stages;  
	
	
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
