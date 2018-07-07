#ifndef __EACSMB_texgen_h__
#define __EACSMB_texgen_h__



#include "common_math.h"
#include "common_gl.h"

#include "hash.h"

#include "gui.h"
#include "texture.h"
#include "perlin.h"
#include "opensimplex.h"


struct TexGenOp;

typedef struct TexGenOp* tgop_ptr;
typedef VEC(struct TexGenOp*) tgop_vec;


#define TEXGEN_TYPE_LIST \
	TEXGEN_TYPE_MEMBER(set) \
	TEXGEN_TYPE_MEMBER(get) \
	TEXGEN_TYPE_MEMBER(seq) \
	TEXGEN_TYPE_MEMBER(context) \
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

typedef struct tg_sampler {
	char type; 
	char source; // 0 - value, 1 = bmp
	char channel; // for bmp source
	
	double dval;
	uint32_t colorval;
	
	FloatTex* bmp;
	
	
} tg_sampler;


typedef struct tg_context {
	char primaryChannel;
	
	HashTable(FloatTex*)* storage;
	
} tg_context;


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
	X(float, offset_x, -999999.0, 9999999.0, 64.0) \
	X(float, offset_y, -999999.0, 9999999.0, 64.0) \
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

#define TG_REFL_STRUCT_NAME context
#define XLIST \
	X(int, primaryChannel, 0, 3, 0) \
	X(tgop_ptr, op, 0, 0, 0) 
#include "tg_reflect.h"
#undef XLIST


#define TG_REFL_STRUCT_NAME seq
#define XLIST \
	X(char_ptr, name, "", "", "") \
	X(tgop_vec, ops, 0, 0, 0) 
#include "tg_reflect.h"
#undef XLIST

#define TG_REFL_STRUCT_NAME set
#define XLIST \
	X(char_ptr, name, "", "", "") \
	X(tgop_ptr, op, 0, 0, 0) 
#include "tg_reflect.h"
#undef XLIST

#define TG_REFL_STRUCT_NAME get
#define XLIST \
	X(char_ptr, name, "", "", "") 
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
