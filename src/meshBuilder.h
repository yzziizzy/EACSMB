#ifndef __EACSMB_meshBuilder_h__
#define __EACSMB_meshBuilder_h__


#include "ds.h"
#include "common_math.h"


typedef struct MeshBuilderVertex {
	Vector v, n;
	struct {
		unsigned short u, v;
	} t;
} MeshBuilderVertex;

typedef VEC(MeshBuilderVertex) MBVList;

// not efficient by any stretch. no CSG capabilities.

typedef struct MeshData {
	MBVList verts;
	VEC(int) indices;
} MeshData;


// DO NOT REORDER
enum MB_op_type {
	MB_OP_NONE = 0,
	MB_OP_COMPOSE,
	MB_OP_TRANSFORM,
	MB_OP_CREATE_CYLINDER,
	MB_OP_CREATE_CUBE,
	MB_OP_CREATE_SPHERE,
	MB_OP_CREATE_PYRAMID
};



typedef union MB_operation MB_operation;


typedef struct {
	enum MB_op_type type;
	VEC(MB_operation) children;
} MB_compose_params;



// TODO: skew, bend, twist, non-linear scaling
typedef struct {
	enum MB_op_type type;
	
	Vector position;
	Vector scale;
	Vector direction;
	float rotation;
	
	VEC(MB_operation) children;
} MB_transform_params;



typedef struct {
	enum MB_op_type type;
	
	float length;
	float radius;
	
	int linear_segments;
	int radial_segments;
	
	char cap_min, cap_max;
} MB_cylinder_params;



union MB_operation {
	enum MB_op_type type;
	MB_compose_params compose;
	MB_transform_params transform;
	MB_cylinder_params cylinder;
};



#endif // __EACSMB_meshBuilder_h__
