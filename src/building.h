#ifndef __EACSMB_building_h__
#define __EACSMB_building_h__


#include "common_math.h"
#include "ds.h"



typedef struct BuildingOutline {
	VEC(Vector2) points;
	char closed;
	
	// mesh info
	int first_index;
	int index_count;
	
	int cap_first_index;
	int cap_index_count;
	
	
} BuildingOutline;



typedef struct Building {
	VEC(BuildingOutline*) outlines;
	
	
	VEC(Vector) vertices;
	VEC(unsigned short) indices;
	
	
} Building;
















#endif // __EACSMB_building_h__
