#include <SDL2/SDL.h>//So everything gets SDL
#include <stdint.h>

#define CHEAT_NUCLEAR 1
#define CHEAT_SLOMO   2

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

typedef struct{
	uint16_t hue;
	uint32_t color;
	int controlMode;
}playerRequest;
