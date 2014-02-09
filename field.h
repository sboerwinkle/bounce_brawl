typedef struct{
	int x, y;
}orderedPair;
extern node* nodes;
extern orderedPair *centers;
extern char* alives;
extern char* injured;
extern int numNodes;
extern tool* tools;
extern int numTools;
extern double maxZoomIn;
extern int zoom;
extern int centerx;
extern int centery;
extern int playerNum;//How many created
extern int markSize;

extern void initField(int xsize, int ysize);

extern void stopField();

extern void killNode(int where);

/**
 * Assumes you'll assign a node to the returned index.
 */
extern int addNode();

/**
 * Assumes you'll assign a tool to the returned index.
 */
extern int addTool();

extern void ensureCapacity(int index);

extern void addTask(task* addme);

/*inline?*/ extern float getScreenX(int x);

/*inline?*/ extern float getScreenY(int y);

extern void run();
