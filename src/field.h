//This structure stores all the information you'd ever want to know about a player, and doubles as the TGC's persistent data!
typedef struct {
	int myNodes[4];
	int controlType;
	int controlIndex;
	//a multi-purpose variable for the current tool.
	int controlVar;
	int lastpress;		// If the connect key was pressed last time
	char lastpressAction;	// ditto for action key
	char *myKeys;
	int num;
	tool *controlData;
	int connectedLeg;
	char exists[4];
	char alive, injured, firstLife;

	double ten0, ten1, nine0, nine1, nine2, eleven0;	// Arm lengths, named after the indices which certain nodes got when I was still testing TGC
	long int respawnx, respawny;
	int centerX, centerY;
} TGCdata;

extern node *nodes;
extern int numNodes;
extern tool *tools;
extern int numTools;
extern double maxZoomIn;
extern int zoom;
extern int centerx;
extern int centery;
extern TGCdata *guyDatas;
extern int playerNum;		//How many created
extern int markSize;

extern void initField();

extern void stopField();

extern void killNode(int where);

extern int addNode();

extern int addTool();

extern void ensureCapacity(int index);

extern float getScreenX(int x);

extern float getScreenY(int y);

extern void run();

extern void draw();
