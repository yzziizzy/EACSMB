#ifndef __EACSMB_ROAD_H__
#define __EACSMB_ROAD_H__


typedef struct RoadVertex {
	Vector2 v;
	struct { float u, v; } t;
} RoadVertex;

typedef struct RoadControlPoint {
	Vector2 cp0;
	Vector2 cp2;
	Vector2 cp1;
} RoadControlPoint;



// dunno what to call this
typedef struct RoadBlock {
	int nextRoad, maxRoads;
	
	RoadControlPoint* cps; 
	
	
} RoadBlock;



void drawRoad(GLuint dtex, Matrix* view, Matrix* proj);





#endif // __EACSMB_ROAD_H__