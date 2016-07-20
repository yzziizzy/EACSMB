

typedef struct TransEdge {
	uint16_t from, to;
	
} TransEdge;

typedef struct TransNode {
	uint16_t type;
	Vector2 pos;
} TransNode;


typedef struct TransGraph {
	int nextNode, nextEdge;
	int maxNode, maxEdge;
	
	TransNode nodes[1<<12];
	TransEdge edges[1<<14];
	
	// probably need a mutex for access
	
} TransGraph;





TransGraph* allocTransGraph();
void initTransGraph(TransGraph* tg);

int addTransNode(TransGraph* tg, uint16_t type, Vector2* pos, uint16_t* out_id);
int addTransEdge(TransGraph* tg, uint16_t from, uint16_t to);













