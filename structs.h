#include <SDL2/SDL.h>//So everything gets SDL

struct task{
        struct task *next;
        void *data;
	Sint8 dataUsed;
        Sint8 (*func) (void *where);
};
typedef struct task task;

typedef struct {
	int type;
	int where;
	char inUse;
} tool;//A tool just labels a particular node (where) as being central to a configuration of nodes and connections which the taskguycontrol can recognize and manipulate for fun and profit

typedef struct {
	Sint8 dead;
	int id;
	double preflength;
	double midlength;
	double tolerance;
	double friction;
	double force;
	Uint8 hue;
} connection;

typedef struct {
	int netIndex;//The index with null nodes not counted. Used for networking.
	Sint8 dead;
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

typedef struct{
	Uint16 hue;
	Uint32 color;
	int controlMode;
}playerRequest;
