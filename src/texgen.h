#ifndef __EACSMB_texgen_h__
#define __EACSMB_texgen_h__



#include "common_math.h"
#include "common_gl.h"


#include "gui.h"
#include "texture.h"
#include "perlin.h"
#include "opensimplex.h"


#define TEXGEN_TYPE_LIST \
	TEXGEN_TYPE_MEMBER(set) \
	TEXGEN_TYPE_MEMBER(get) \
	TEXGEN_TYPE_MEMBER(seq) \
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
	X(float, spread_x, 0.00001, 9999999.0, 64.0) \
	X(float, spread_y, 0.00001, 9999999.0, 64.0) \
	X(float, persistence, 0.00001, 9999999.0, 0.1) \
	X(int, octaves, 0, 99, 5) 
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

struct TexGenOp;


struct TG_set {
	char* name;
	struct TexGenOp* op;
};

struct TG_get {
	char* name;
};


struct TG_seq {
	VEC(struct TexGenOp*) ops;
};


typedef struct TexGenOp {
	TexGenType type;
	int channel_out;
	union {		
		#define TEXGEN_TYPE_MEMBER(x) struct TG_##x x;
			TEXGEN_TYPE_LIST
		#undef TEXGEN_TYPE_MEMBER
	};
} TexGenOp;


typedef struct TexGenContext {
	
	HashTable* stages;  
	
	
	Texture* output;
	
} TexGenContext;







typedef struct GUITexBuilderControl {
	GUIHeader header;
	
	GUISimpleWindow* bg;
	GUIImage* im;
	
	TexGenContext* tg;
	
	InputEventHandler* inputHandlers;
	
	
} GUITexBuilderControl;






GUITexBuilderControl* guiTexBuilderControlNew(Vector2 pos, Vector2 size, int zIndex);










#endif // __EACSMB_texgen_h__
