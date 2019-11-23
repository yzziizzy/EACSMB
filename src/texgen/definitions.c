
#include "gui.h"


#define TG_DEFINE_REFLECTION
#include "texgen.h"

#include "core.h"


char* texgen_op_names[] = {
#define TEXGEN_TYPE_MEMBER(x) [TEXGEN_TYPE_##x] = #x,
	TEXGEN_TYPE_LIST
#undef TEXGEN_TYPE_MEMBER
};


GUISA_Field* tgsa_fields[] = {
#define TEXGEN_TYPE_MEMBER(x) [TEXGEN_TYPE_##x] = TG_ ## x ## _structAdjusterFields,
	TEXGEN_TYPE_LIST
#undef TEXGEN_TYPE_MEMBER
};



// forward declarations
#define TEXGEN_TYPE_MEMBER(x) static void gen_##x(struct tg_context* context, struct TG_##x* opts);
	TEXGEN_TYPE_LIST
#undef TEXGEN_TYPE_MEMBER



genfn generators[] = {
#define TEXGEN_TYPE_MEMBER(x) [TEXGEN_TYPE_##x] = (genfn)gen_##x,
	TEXGEN_TYPE_LIST
#undef TEXGEN_TYPE_MEMBER
};



#define TEXGEN_TYPE_MEMBER(x) \
	static void CAT_(setDefault_, CAT_(TG_, x))(struct TexGenOp* o) { \
		o->x = CAT_(CAT_(TG_, x), _defaultValue); \
	}
	
	TEXGEN_TYPE_LIST
#undef TEXGEN_TYPE_MEMBER


setDefaultFn setDefaultsFns[] = {
#define TEXGEN_TYPE_MEMBER(x) [TEXGEN_TYPE_##x] = (void*)setDefault_TG_##x,
	TEXGEN_TYPE_LIST
#undef TEXGEN_TYPE_MEMBER
};

