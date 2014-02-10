#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "structs.h"
#include "gfx.h"
#include "node.h"
#include "gui.h"
#include "task.h"
#include "networking.h"
#include <math.h>

typedef struct{
	int x, y;
}orderedPair;
node* nodes;
orderedPair *centers;
char* alives;
char* injured;
static task* firstTask;
int numNodes;
static char *corpses;//prevents a node from being placed in the spot of a recently slain node, preventing confusion.
tool *tools;
int numTools;
static node *current;//All these are global so they aren't reallocated every tick. Not sure how useful that is, but whatever.
static node *current2;
static double deltaX;
static double deltaY;
static double forceNewY;
static double currlengthNewX;
static double unx;
static double uny;
double maxZoomIn;
int zoom;
static int width;
static int height;

int playerNum;
int centerx;
int centery;
int markSize;//Invented for networking, but also used by task.c to do player marks.

void initField(int xsize, int ysize){// Here: init taskguycontrolindexes, alives, centers to proper number of players
	width = xsize;
	height = ysize;
	numNodes = 100;
	nodes = (node*)malloc(numNodes*sizeof(node));
	int i = 0;
	for(; i < numNodes; i++){
		nodes[i].dead = 1;
		nodes[i].netIndex = -1;
	}
	corpses = (char*)calloc(numNodes, sizeof(char));
	firstTask = NULL;
	tools = NULL;
	numTools = 0;
	playerNum = 0;
	maxZoomIn = 2.25;
	zoom = 1;
	taskguycontrolindexes = malloc(sizeof(int)*players);
	alives = malloc(players);
	injured = malloc(players);
	for(i = 0; i < players; i++) injured[i] = 0;
	centers = malloc(sizeof(orderedPair)*players);
}

void stopField(){
	register int i;
	node* iterator = nodes;
	for(i = 0; i < numNodes; i++, iterator++){
		if(iterator->dead) continue;
		free(iterator->connections);
	}
	numNodes = 0;
	free(taskguycontrolindexes);
	free(alives);
	free(injured);
	free(centers);
	free(nodes);			//If Heaven and Hell
	free(corpses);			//Are both satisfied
	free(tools);			//Then I'll follow you
	freeAllTasks(firstTask);	//Into the dark
}

void killNode(int where){
	nodes[where].dead = 1;
	corpses[where] = 2;
	free(nodes[where].connections);
	if(netMode){
		for(; where < numNodes; where++) nodes[where].netIndex--;
	}
}

/**
 * Assumes you'll assign a node to the returned index.
 */
int addNode(){
	int i = 0;
	for(; i < numNodes; i++){
		if(nodes[i].dead && corpses[i] == 0){
			if(netMode){
				int j = i;
				for(; j < numNodes; j++) nodes[j].netIndex++;
			}
			return i;
		}
	}
	printf("Expanding capacity to %d\n",numNodes+=25);
	nodes = realloc(nodes, numNodes*sizeof(node));
	corpses = realloc(corpses, numNodes*sizeof(char));
	corpses[i] = 0;
	int j = i + 1;
	for(; j < numNodes; j++){
		corpses[j] = 0;
		nodes[j].dead = 1;
	}
	if(netMode){
		int index = nodes[i-1].netIndex+1;
		for(j = i; j < numNodes; j++){
			nodes[j].netIndex = index;
		}
	}
	return i;
}
/**
 * Assumes you'll assign a tool to the returned index.
 */
int addTool(){
	int i = 0;
	for(; i < numTools; i++){
		if(tools[i].where == -1) return i;
	}
	numTools++;
//	printf("Expanding tool capacity to %d\n",numTools);
	tools = (tool*)realloc(tools, numTools*sizeof(tool));
	return i;
}								//Lyrics played from the shower:
void ensureCapacity(int index){					//Just gonna stand there and watch me burn
	if(index < numNodes) return;				//That's alright because I like the way it hurts
	index++;						//Just gonna stand there and hear me cry
	nodes = realloc(nodes, index*sizeof(node));		//That's alright because I love the way you lie
	corpses = realloc(corpses, index*sizeof(char));	//I love the way you lie
	int i = numNodes;					//Ohhh
	for(; i < index; i++){					//I love the way you lie
		nodes[i].dead = 1;
		corpses[i] = 0;
	}
	if(netMode){
		int index = nodes[numNodes-1].netIndex;
		for(i = numNodes; i < index; i++) nodes[i].netIndex = index;
	}
	numNodes = index;
}
void addTask(task* addme){
	addme->next = firstTask;
	firstTask = addme;
}

float getScreenX(int x){
	return (float)x/zoom/width2;
}
float getScreenY(int y){
	return (float)y/zoom/height2;
}

void run(){
//	SDL_SetRenderDrawColor(screen, 0, 0, 0, 255);
//	SDL_Rect a = {.x=0, .y=0, .w=500, .h=500};
//	SDL_RenderFillRect(screen, &a);
	{//Since some of the tasks draw text and drawing text uses register variables, these curly variables ensure the next two register variables go out of scope early enough. Or at least that's the goal.
		register int i;
		register int j;
		//decay the corpses
		for(i = 0; i < numNodes; i++){
			if(corpses[i] != 0) corpses[i]--;
		}
		//springs
		double* frictionXs = calloc(sizeof(double), numNodes);
		double* frictionYs = calloc(sizeof(double), numNodes);
		for(i = 0; i < numNodes; i++){
			if(nodes[i].dead){continue;}
			current = nodes+i;
			for(j=0; j<current->numConnections; j++){
				if(current->connections[j].dead){continue;}
				if(nodes[current->connections[j].id].dead){
					current->connections[j].dead=1;
					continue;
				}
				current2 = nodes + current->connections[j].id;
				deltaX = current->x - current2->x + current->px - current2->px;
				deltaY = current->y - current2->y + current->py - current2->py;
				currlengthNewX = sqrt(deltaX*deltaX + deltaY*deltaY);
				if(fabs(currlengthNewX - current->connections[j].midlength) > current->connections[j].tolerance){
					current->connections[j].dead = 1;
					continue;
				}
				current->connections[j].hue = (uint8_t)(fabs(currlengthNewX - current->connections[j].midlength)/current->connections[j].tolerance*256);
				unx = deltaX / currlengthNewX;
				uny = deltaY / currlengthNewX;
				deltaY = current->connections[j].friction*((current->xmom - current2->xmom)*unx + (current->ymom - current2->ymom)*uny);
					//deltaY = current->connections[j].friction*deltaY + current->connections[j].force*(current->connections[j].preflength - currlengthNewX);
				forceNewY = deltaY * current2->mass / (current->mass + current2->mass);
				frictionXs[i] += forceNewY*unx;
				frictionYs[i] += forceNewY*uny;
				forceNewY -= deltaY;
				frictionXs[current->connections[j].id] += forceNewY*unx;
				frictionYs[current->connections[j].id] += forceNewY*uny;
				forceNewY = current->connections[j].force*(current->connections[j].preflength - currlengthNewX);
				current->xmom += forceNewY*unx/current->mass;
				current->ymom += forceNewY*uny/current->mass;
				current2->xmom -= forceNewY*unx/current2->mass;
				current2->ymom -= forceNewY*uny/current2->mass;
			}
		}
		for(i = 0; i < numNodes; i++){
			if(nodes[i].dead) continue;
			nodes[i].xmom += frictionXs[i];
			nodes[i].ymom += frictionYs[i];
		}
		free(frictionXs);
		free(frictionYs);
		//collision detection (technically flawed)
		char* check = calloc(numNodes, sizeof(char));
		char* checkNext = calloc(numNodes, sizeof(char));
		char firstTime = 1;
		char collided;
		char checkall;
		node *he;
		node *I;
		int loopcounter = 0;
		while(1){//Loop at most 5 times
			collided = 0;
			for(i = 0; i < numNodes; i++){
				if(nodes[i].dead) continue;
				I = &nodes[i];
				checkall = check[i] || firstTime;
				for(j = i+1; j < numNodes; j++){
					if(nodes[j].dead) continue;
					if(!(checkall || check[j])) continue;
					he = nodes+j;
					deltaX = I->x - he->x + I->px - he->px;
					deltaY = I->y - he->y + I->py - he->py;
					currlengthNewX = sqrt(deltaX*deltaX + deltaY*deltaY);//consider using newtons method: n+1 = (n + x/n)/2;
					if(currlengthNewX > I->size + he->size || currlengthNewX == 0) continue;
					unx = deltaX / currlengthNewX;
					uny = deltaY / currlengthNewX;
					deltaY = 1.333*((I->xmom - he->xmom)*unx + (I->ymom - he->ymom)*uny);//constant = bounciness
					if(deltaY >= 0) continue;
					//Ymom -= 1*(I.mass+he.mass)*(I.size+he.size - currlengthNewX);
					checkNext[i] = 1;
					checkNext[j] = 1;
					collided = 1;
					forceNewY = deltaY * -he->mass / (I->mass + he->mass);
					I->xmom += forceNewY*unx;
					I->ymom += forceNewY*uny;
					forceNewY += deltaY;
					he->xmom += forceNewY*unx;
					he->ymom += forceNewY*uny;
				}
			}
			if(!collided || ++loopcounter >= 5) break;
			firstTime = 0;
			free(check);
			check = checkNext;
			checkNext = calloc(numNodes, sizeof(char));
		}
		free(check);
		free(checkNext);
		for(i = 0; i < numNodes; i++){
			if(nodes[i].dead){continue;}
			positioncleanup(&nodes[i]);
		}
	}

	//start drawing
	centerx = 0;
	centery = 0;
	int i, j;
	int count = 0;
	for(i = 0; i < 2; i++){
		if(pIndex[i] != -1 && pIndex[i] < players && alives[pIndex[i]]){
			count++;
			centerx += centers[pIndex[i]].x;
			centery += centers[pIndex[i]].y;
		}
	}
	if(count != 0){
		centerx *= maxZoomIn/count;
		centery *= maxZoomIn/count;
	}/*else{
		centerx = width2;
		centery = height2;
	}*/

	int x, y, numActiveCons;
	int netNumCons = 0;
	for(i = 0; i < numNodes; i++){
		if(nodes[i].dead){continue;}
		current = &nodes[i];
		int size = (int)(current->size*maxZoomIn);
		if(size != 0){
			setColorWhite();
			x = current->x*maxZoomIn;
			y = current->y*maxZoomIn;
			drawCircle(getScreenX(x-centerx), getScreenY(y-centery), (float)size/zoom/width2);
			if(netMode)
				addNetCircle(x, y, size);
		}
		if(netMode && current->numConnections != 0){
			numActiveCons = 0;
			netNumCons = addNetLineCircle(current->netIndex);
		}
		for(j = 0; j < current->numConnections; j++){
			if(current->connections[j].dead) continue;
			if(netMode){
				numActiveCons++;
				addNetLine(nodes[current->connections[j].id].netIndex, current->connections[j].hue);
			}
			setColorFromHue(current->connections[j].hue);
			drawLine(getScreenX(current->x*maxZoomIn-centerx), getScreenY(current->y*maxZoomIn-centery), getScreenX(nodes[current->connections[j].id].x*maxZoomIn-centerx), getScreenY(nodes[current->connections[j].id].y*maxZoomIn-centery));
		}
		if(netMode && current->numConnections != 0){
			if(numActiveCons) setNetLineCircleNumber(netNumCons, numActiveCons);
			else removeNetLineCircle();
		}
	}
	setColorWhite();
	float a, b, c;
	markSize = (int)(maxZoomIn*6)/zoom;
	c = (float)markSize/width2/2;
	if(markSize < 2) markSize = 2;
	for(i = 0; i < numTools; i++){
		if(tools[i].where == -1) continue;
		node* tool = nodes+tools[i].where;
		if(tool->dead){
			tools[i].where = -1;
		}else{
			a = getScreenX((tool->x)*maxZoomIn - centerx);
			b = getScreenY((tool->y)*maxZoomIn - centery);
			setColorFromHex(getToolColor(tools[i].type));
			drawRectangle(a-c, b+c, a+c, b-c);
			if(netMode) addNetTool(tool->netIndex, tools[i].type);
		}
	}
	runTask(&firstTask);//A linked list. This function runs them in order and snips out dead ones, freeing them.
}
