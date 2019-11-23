

#ifndef TG_REFLECT_DEF
	#define TG_REFLECT_DEF

#endif // TG_REFLECT_DEF


#define CAT_INNER(a,b) a ## b
#define CAT_(a,b) CAT_INNER(a,b)


#define decl_Vector4 Vector4
#define decl_float float
#define decl_int int
#define decl_unsigned_int unsigned int
#define decl_char_ptr char*
#define decl_tgop_ptr struct TexGenOp*
#define decl_tgop_vec VEC(struct TexGenOp*)

#define decl_typ(_type) CAT_(decl_, _type) 


// fed into StructAdjuster/DebugAdjuster as the type 
#define declc_Vector4 'f'
#define declc_float 'f'
#define declc_int 'i'
#define declc_unsigned_int '4'
#define declc_char_ptr 'a'
#define declc_tgop_ptr 'p'
#define declc_tgop_vec 'p'

#define declc_typ(_type) CAT_(declc_, _type) 



#define declclen_Vector4 4
#define declclen_float 1
#define declclen_int 1
#define declclen_unsigned_int 1
#define declclen_char_ptr 1
#define declclen_tgop_ptr 1
#define declclen_tgop_vec 1

#define declclen_typ(_type) CAT_(declclen_, _type) 

typedef struct CAT_(TG_, TG_REFL_STRUCT_NAME) {
#define X(_type, _name, _min, _max, _def) decl_typ(_type) _name; 
	XLIST
#undef X
} CAT_(TG_, TG_REFL_STRUCT_NAME);



#define init_Vector4(z) { .v4 = COLOR_TO_VEC4(z) }
#define init_float(z) { .f = (z) }
#define init_int(z) { .i = (z) }
#define init_unsigned_int(z) { .u = (z) }
#define init_char_ptr(z) { .s = z }
#define init_tgop_ptr(z) { .f = z }
#define init_tgop_vec(z) { .f = z }

#define init_val(type, z) init_##type(z) 


#define init2_Vector4(z) COLOR_TO_VEC4(z)
#define init2_float(z) (z)
#define init2_int(z) (z)
#define init2_unsigned_int(z) (z)
#define init2_char_ptr(z) z
#define init2_tgop_ptr(z) z
#define init2_tgop_vec(z) z

#define init2_val(type, z) init2_##type(z) 


// only one of these needs to exist somewhere
#ifdef TG_DEFINE_REFLECTION

	struct tg_reflect CAT_(CAT_(TG_, TG_REFL_STRUCT_NAME), _ref)[] = {
	#define X(_type, _name, _min, _max, _def) { \
		.name = #_name, \
		.member_offset = offsetof(struct CAT_(TG_, TG_REFL_STRUCT_NAME), _name), \
		.type = 1, \
		.range_min = init_val(_type, _min), \
		.range_max = init_val(_type, _max), \
		.def_value = init_val(_type, _def), \
	},
		
		XLIST
	#undef X
	};
	
	// for the gui struct adjuster
	
	struct GUISA_Field CAT_(CAT_(TG_, TG_REFL_STRUCT_NAME), _structAdjusterFields)[] = {
	#define X(_type, _name, _min, _max, _def) { \
		.name = #_name, \
		.base = 0, \
		.offset = offsetof(struct CAT_(TG_, TG_REFL_STRUCT_NAME), _name), \
		.type = declc_typ(_type), \
		.count = declclen_typ(_type), \
		.formatSuffix = NULL, \
	},
		
		XLIST
		{NULL},
	#undef X
	};
	
	
	struct CAT_(TG_, TG_REFL_STRUCT_NAME) 
	CAT_(CAT_(TG_, TG_REFL_STRUCT_NAME), _defaultValue) = {
	#define X(_type, _name, _min, _max, _def) ._name = init2_val(_type, _def),
		XLIST
	#undef X
	};
	
	
#else
	// everywhere else is declared extern
	struct tg_reflect;
	struct GUISA_Field;
	
	extern struct tg_reflect CAT_(CAT_(TG_, TG_REFL_STRUCT_NAME), _ref)[];
	extern struct GUISA_Field CAT_(CAT_(TG_, TG_REFL_STRUCT_NAME), _structAdjusterFields)[];
	extern struct CAT_(CAT_(TG_, TG_REFL_STRUCT_NAME), _defaultValue);
	
#endif




#undef init_val
#undef init_Vector4
#undef init_float
#undef init_int
#undef init_unsigned_int
#undef init_char_ptr


#undef decl_type
#undef decl_Vector4
#undef decl_float
#undef decl_int
#undef decl_unsigned_int
#undef decl_char_ptr

#undef TG_REFL_STRUCT_NAME
