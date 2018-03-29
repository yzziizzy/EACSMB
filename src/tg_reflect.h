

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

#define decl_typ(_type) CAT_INNER(decl_, _type) 

typedef struct CAT_(TG_, TG_REFL_STRUCT_NAME) {
#define X(_type, _name, _min, _max, _def) decl_typ(_type) _name; 
	XLIST
#undef X
}; CAT(TG_, TG_REFL_STRUCT_NAME);



// vector4 doesn't have a real initializer. this just prevents compile errors
#define init_Vector4(z) { .f = (z) }
#define init_float(z) { .f = (z) }
#define init_int(z) { .i = (z) }
#define init_unsigned_int(z) { .u = (z) }
#define init_char_ptr(z) { .s = z }

#define init_val(type, z) init_##type(z) 


// only one of these needs to exist somewhere
#ifndef  CAT_(CAT_(TG_, TG_REFL_STRUCT_NAME), _ref_DEFINED)
	#define CAT_(CAT_(TG_, TG_REFL_STRUCT_NAME), _ref_DEFINED)


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
#else
	// everywhere else is declared extern
	extern struct tg_reflect CAT_(CAT_(TG_, TG_REFL_STRUCT_NAME), _ref)[];

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
