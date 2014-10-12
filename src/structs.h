#include <SDL2/SDL.h>		//So everything gets SDL
#include <stdint.h>

//This number defines how much of a "base" tick it actually computes each tick. The number of ticks per second is scaled accordingly, so interactions in the game should take approximately the same amount of time. I'm fairly sure I've tied everything of importance to this number, but it's still possible there's some value, somewhere, which doesn't scale and things will actually behave fundamentally differently.
//Barring such an occurance, the smaller this number, the better the resolution of your physics, and the less often you'll get a foot stuck in the flooring.
//The larger this number, the more CPU friendly.
//I should mention that things were tested at this value, so changing it might, in some situations, produce different results. Things might break, but I wouldn't count on it.
#define SPEEDFACTOR 0.7
//After you reduce SPEEDFACTOR past a point (on my computer, at least), you'll notice the CPU isn't at full potential but you're still getting lag. At that point it's probably time to increase this number, which determines what fraction of frames computed are actually shown. By default, this value is 2, meaning every second frame is drawn. This reduces the load of your graphics card.
//Also note increasing this number will reduce network traffic when you're hosting, as I take a fairly simple, send-info-describing-the-frame approach to networking.
#define SHOWEVERYNTHFRAME 2

#ifdef WINDOWS
	#define srandom srand
	#define random rand
#endif

typedef struct task {
	//Which task is next in the linked list
	struct task *next;
	//Where our data is stored, if we have any
	void *data;
	//Whether we have any
	char dataUsed;
	//Which funcion to call, and pass our data to
	char (*func) (void *where);
} task;

//A tool just labels a particular node (where) as being central to a configuration of nodes and connections which the taskguycontrol can recognize and manipulate for fun and profit
typedef struct {
	//What kind of tool
	int type;
	//Which node it's centered on
	int where;
	//Whether someone (alive) is connected to it
	char inUse;
} tool;

typedef struct {
	//If the connection is snapped
	char dead;
	//Who the connection is connected to
	int id;
	//Which length it wants to get to
	double preflength;
	//Which length actually puts it under the least stress
	double midlength;
	//How far it can stray from midlength w/out snapping
	double tolerance;
	//What fraction of the nodes' relative motion towards or away from each other we permit to remain. It's actually stored as 1 - that value, but it's passed to the initializer functions as described. Note that too many connections with aggressive frictions may cause... crazy results.
	//Usage: Pass 1.0 for regular bands, approx. 0.85 - 0.99 for reasonable degrees of damping.
	double friction;
	//Multiplied by our displacement from preflength to give actual force applied.
	double force;
	//What color to draw it as
	uint8_t hue;
} connection;

typedef struct {
	//The index with null nodes not counted. Used for networking.
	int netIndex;
	//True if the node was destroyed
	char dead;
	long int x;
	long int y;
	//Precise x and y
	double px;
	double py;
	//Actually velocity
	double xmom;
	double ymom;
	double size;
	double mass;
	connection *connections;
	int numConnections;
} node;
