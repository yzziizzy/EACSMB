#ifndef __EACSMB_building_h__
#define __EACSMB_building_h__


#include "common_math.h"
#include "ds.h"

#include "dynamicMesh.h"


typedef struct BuildingOutline {
	VEC(Vector2) points;
	char closed;
	
	float extruded_height;
	
	// mesh info
	int first_index;
	int index_count;
	
	int cap_first_index;
	int cap_index_count;
	
	
} BuildingOutline;



typedef struct Building {
	VEC(BuildingOutline*) outlines;
	
	VEC(Vertex_PNT) vertices;
	VEC(unsigned short) indices;
	
	
} Building;




void Building_extrudeAll(Building* b, float height);
void Building_extrudeOutline(Building* b, BuildingOutline* o, float height);

// flat cap
void Building_capAll(Building* b, float height);
void Building_capOutline(Building* building, BuildingOutline* o, float height);

void Building_pointCapOutline(Building* building, BuildingOutline* o, float height, float capHeight);


DynamicMesh* Building_CreateDynamicMesh(Building* b);




#endif // __EACSMB_building_h__
