#include <SDL2/SDL.h>//So everything gets SDL
#include <stdint.h>

//This number defines how much of a "base" tick it actually computes each tick. The number of ticks per second is scaled accordingly, so interactions in the game should take approximately the same amount of time. I'm fairly sure I've tied everything of importance to this number, but it's still possible there's some value, somewhere, which doesn't scale and things will actually behave fundamentally differently.
//Barring such an occurance, the smaller this number, the better the resolution of your physics, and the less often you'll get a foot stuck in the flooring.
//The larger this number, the more CPU friendly.
//I should mention that things were tested at this value, so changing it might, in some situations, produce different results. Things might break, but I wouldn't count on it.
#define SPEEDFACTOR 0.7
//After you reduce SPEEDFACTOR past a point (on my computer, at least), you'll notice the CPU isn't at full potential but you're still getting lag. At that point it's probably time to increase this number, which determines what fraction of frames computed are actually shown. By default, this value is 2, meaning every second frame is drawn. This reduces the load of your graphics card.
//Also note increasing this number will reduce network traffic when you're hosting, as I take a fairly simple, send-info-describing-the-frame approach to networking.
#define SHOWEVERYNTHFRAME 2

struct task{
        struct task *next;
        void *data;
	char dataUsed;
        char (*func) (void *where);
};
typedef struct task task;

typedef struct {
	int type;
	int where;
	char inUse;
} tool;//A tool just labels a particular node (where) as being central to a configuration of nodes and connections which the taskguycontrol can recognize and manipulate for fun and profit

typedef struct {
	char dead;
	int id;
	double preflength;
	double midlength;
	double tolerance;
	double friction;
	double force;
	uint8_t hue;
} connection;

typedef struct {
	int netIndex;//The index with null nodes not counted. Used for networking.
	char dead;
	long int x;
	long int y;
	double px;
	double py;
	double xmom;
	double ymom;
	double size;
	double mass;
	connection* connections;
	int numConnections;
} node;
