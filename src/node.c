#include <stdlib.h>
#include "structs.h"
#include "field.h"
#include <math.h>

void newConnectionLong(int node, int con, int id, double fric,
		       double prefLen, double mid, double tol,
		       double force)
{
	connection *who = nodes[node].connections + con;
	who->dead = 0;
	who->id = id;
	who->preflength = prefLen;
	who->midlength = mid;
	who->tolerance = tol;
	who->friction = pow(fric, SPEEDFACTOR) - 1;
	who->force = force * SPEEDFACTOR;
}

void newConnection(int node, int con, int id, double fric, double len,
		   double tol, double force)
{
	newConnectionLong(node, con, id, fric, len, len, tol, force);
}

int newNodeLong(long X, long Y, double Px, double Py, double Xm, double Ym,
		double S, double M, int array)
{
	int ret = addNode();
	node *who = nodes + ret;
	who->dead = 0;
	who->x = X;
	who->y = Y;
	who->px = Px;
	who->py = Py;
	who->xmom = Xm;
	who->ymom = Ym;
	who->size = S;
	who->mass = M;
	who->numConnections = array;
	who->connections =
	    (connection *) malloc(array * sizeof(connection));
	return ret;
}

int newNode(int x, int y, double size, double mass, int array)
{
	return newNodeLong((long) x, (long) y, (double) 0, (double) 0, 0,
			   0, size, mass, array);
}

//Most fucntions are called addSomething, but this one is 'create' as a reminder that this shouldn't be called once the simulation starts. Doesn't clean out dead connections.
int createConnection(int who)
{
	nodes[who].connections =
	    (connection *) realloc(nodes[who].connections,
				   (++nodes[who].numConnections) *
				   sizeof(connection));
	return nodes[who].numConnections - 1;
}

void positioncleanup(node * who)
{
	who->px += who->xmom;
	int n = floor(who->px);
	who->x += n;
	who->px -= n;
	who->py += who->ymom;
	n = floor(who->py);
	who->y += n;
	who->py -= n;
}
