#ifndef __EACSMB_building_h__
#define __EACSMB_building_h__


#include "common_math.h"
#include "ds.h"

#include "dynamicMesh.h"



LIST_DECL(Vector2, point);





typedef struct BuildingOutline {
	Vector2_List points;
	char closed;
	
	float h_offset;
	float extruded_height;
	
	float cap_height;
	
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




BuildingOutline* BuildingOutline_alloc(float h_offset, float ex_height);
BuildingOutline* BuildingOutline_rect(float h_offset, float ex_height, Vector2 pos, Vector2 size);



void Building_extrudeAll(Building* b);
void Building_extrudeOutline(Building* b, BuildingOutline* o);

// flat cap
void Building_capAll(Building* b);
void Building_capOutline(Building* building, BuildingOutline* o);

void Building_pointCapOutline(Building* building, BuildingOutline* o);


DynamicMesh* Building_CreateDynamicMesh(Building* b);




#endif // __EACSMB_building_h__
