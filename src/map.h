#ifndef __map_h__
#define __map_h__



#define MAP_SZ 32


#define MAP_IDX_MASK(x,y) (((1 << 2) << (x)) & (1 << (y)))

struct MapBlock;



typedef struct MapNode {
	
	short dataStride;
	
	unsigned char level;
	unsigned char usage;
	
	int x, y; // center coords
	struct MapNode* parent;
	
	union {
		struct MapNode* kids[2][2];
		struct MapBlock* blocks[2][2];
	};
} MapNode;


typedef struct MapBlock {
	void* data;
	int cx, cy;
	int counter;
	struct MapBlock* xp, *xm, *yp, *ym;
} MapBlock;




// MapBlock* allocRootMapNode(short stride);







void printMapMemoryStats();



#endif // __map_h__