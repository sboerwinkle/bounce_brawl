typedef struct{
	int myNodes[4];
	int controlType;
	int controlIndex;
	//int controlvar;//Currently unused, but intended as a multi-purpose variable for the current tool.
	int lastpress; // If the connect key was pressed last time
	char lastpressAction; // ditto for action key
	char* myKeys;
	int num;
	tool* controlData;
	int connectedLeg;
	char exists[4];
	char alive, injured, firstLife;

	double ten0, ten1, nine0, nine1, nine2, eleven0; // Arm lengths, named after the indices which certain nodes got when I was still testing taskguycontrol
	long int respawnx, respawny;
	int centerX, centerY;
}taskguycontroldata;

extern node* nodes;
extern int numNodes;
extern tool* tools;
extern int numTools;
extern double maxZoomIn;
extern int zoom;
extern int centerx;
extern int centery;
extern taskguycontroldata* guyDatas;
extern int playerNum;//How many created
extern int markSize;

extern void initField();

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

/*inline?*/ extern float getScreenX(int x);

/*inline?*/ extern float getScreenY(int y);

extern void run();

extern void draw();
