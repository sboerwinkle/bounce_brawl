#include <stdlib.h>
#ifdef WINDOWS
	#include <stdio.h>
	#include <string.h>
#endif
#include "structs.h"
#include "gui.h"
#include "field.h"
#include "node.h"
#include "font.h"
#include "gfx.h"
#include "networking.h"
#include <math.h>

task* firstTask;

void addTask(task* addme){
	addme->next = firstTask;
	firstTask = addme;
}

void runTask(task **where){
	while(*where != NULL){
		if( (*(*where)->func) ((*where)->data)){
			task *temp = *where;
			*where = temp->next;
			free(temp);
		}else where = &(*where)->next;
	}
}

void freeAllTasks(task* where){
	task* next;
	while(where != NULL){
		next = where->next;
		if(where->dataUsed) free(where->data);
		free(where);
		where = next;
	}
}

int* taskguycontrolindexes;

typedef struct {
	int target;
	int player;
	char mode;
	int cycle;
	char notDiehard;
	char* myKeys;
} taskaidata;

static char taskaibasiccycle(taskaidata* data, int delay){
	if(!guyDatas[data->player].alive || (data->notDiehard && guyDatas[data->player].injured)) return 2;
	if(data->cycle-- == 0){
		data->cycle = delay;
		data->mode = !data->mode;
		int i = 0;
		for(; i < NUMKEYS; i++) data->myKeys[i]=0;
		if(data->mode){
			if(guyDatas[data->target].injured){
				int targets[players];
				int numTargets = 0;
				int i = 0;
				for(; i < players; i++){
					if(!guyDatas[i].injured && i != data->player)
						targets[numTargets++] = i;
				}
				if(numTargets == 0) return 2;//We're the last one standing
				data->target = targets[rand()%numTargets];
			}
			return 1;
		}
	}
	return 0;
}

static char taskaicombat(void* where){
	taskaidata* data = (taskaidata*)where;
	int* myNodes = guyDatas[data->player].myNodes;
	char ret = taskaibasiccycle(data, 10/SPEEDFACTOR);
	if(ret == 2){
		free(where);
		return 1;
	}
	if(ret == 0) return 0;

	char dir = guyDatas[data->target].centerX > guyDatas[data->player].centerX;
	long maxHeight = 0;
	int ix = -1;
	int i = 0;
	for(; i < 4; i++){
		if(nodes[myNodes[i]].dead) continue;
		long fitness = nodes[myNodes[i]].y+(dir?-nodes[myNodes[i]].x:nodes[myNodes[i]].x);
		if(fitness > maxHeight || ix == -1){
			ix = i;
			maxHeight = fitness;
		}
	}
	data->myKeys[ix] = 1;
	return 0;
}

void taskaicombatadd(int Player, char diehard){
	if(players < 2) return;
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskaicombat;
	taskaidata* data = (taskaidata*)malloc(sizeof(taskaidata));
	current->dataUsed = 1;
	current->data = data;
	int target = rand()%(players-1);
	if(target >= Player) target++;
	data->target = target;
	data->cycle = 5;
	data->player = Player;
	data->mode = 1;
	data->myKeys = masterKeys + NUMKEYS*Player;
	data->notDiehard = !diehard;
	addTask(current);
}

static char taskaispacecombat(void* where){
	taskaidata* data = (taskaidata*)where;
	char ret = taskaibasiccycle(data, 25/SPEEDFACTOR);
	if(ret == 2){
		free(where);
		return 1;
	}
	if(ret == 0) return 0;

	int dx = guyDatas[data->target].centerX - guyDatas[data->player].centerX;
	int dy = guyDatas[data->target].centerY - guyDatas[data->player].centerY;
	node* current;
	int* myNodes = guyDatas[data->player].myNodes;
	double dist, bestDist = 100000;
	double tmpdx, tmpdy, bestdx = 0, bestdy = 0;
	int avgx = guyDatas[data->player].centerX;
	int avgy = guyDatas[data->player].centerY;
	int ix = 0, i;
	for(i=0; i<4; i++){
		if(!guyDatas[data->player].exists[i]) continue;
		current = nodes+myNodes[i];
		tmpdx = current->x+current->px-avgx;
		tmpdy = current->y+current->py-avgy;
		dist = tmpdx*tmpdx + tmpdy*tmpdy;
		if(dist < bestDist){
			bestDist = dist;
			bestdx = tmpdx;
			bestdy = tmpdy;
			ix = i;
		}
	}
	if(bestdx*dx+bestdy*dy > 0){ // If the dot product is positive, the selected foot is pointing towards the target. Fix that.
		if(bestdx*dy > bestdy*dx) ix = (ix+3)%4;
		else ix = (ix+1)%4;
	}
	data->myKeys[ix] = 1;
	return 0;
}

void taskaispacecombatadd(int Player, char diehard){
	if(players < 2) return;
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskaispacecombat;
	taskaidata* data = (taskaidata*)malloc(sizeof(taskaidata));
	current->dataUsed = 1;
	current->data = data;
	int target = rand()%(players-1);
	if(target >= Player) target++;
	data->target = target;
	data->cycle = 5;
	data->player = Player;
	data->mode = 1;
	data->myKeys = masterKeys + NUMKEYS*Player;
	data->notDiehard = !diehard;
	addTask(current);
}

typedef struct {
	int size;
	int mass;
	int cool;
	int maxcool;
} taskasteroidsdata;
static char taskasteroids(void* where){
	taskasteroidsdata* data = (taskasteroidsdata*)where;
	if(data->cool-- <= 0){
		newNodeLong((float)rand()/RAND_MAX*400, 0, 0, 0, (float)rand()/RAND_MAX*4-2, 0, data->size, data->mass, 0);
		data->cool = data->maxcool;
	}
	return 0;
}
void taskasteroidsadd(int s, int m, int c){
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskasteroids;
	taskasteroidsdata* data = (taskasteroidsdata*)malloc(sizeof(taskasteroidsdata));
	current->dataUsed = 1;
	current->data = data;
	data->size = s;
	data->mass = m;
	data->maxcool = c/SPEEDFACTOR;
	data->cool = data->maxcool;
	addTask(current);
}

static char taskcenter(void* where){
	int* index = (int*)where;//'*index' is the size, index[n] is the nth element, 1 indexed.
	register int i = 1;
	long dx = 0;
	long dy = 0;
	int dxv = 0;
	int dyv = 0;
	while(i <= *index){
		if(nodes[index[i]].dead){
			index[i] = index[*index];
			(*index)--;
			continue;
		}
		dx -= nodes[index[i]].x;
		dy -= nodes[index[i]].y;
		dxv -= nodes[index[i]].xmom;
		dyv -= nodes[index[i]].ymom;
		i++;
	}
	if(*index == 0){
		free(where);
		return 1;
	}
	dx /= *index;
	dy /= *index;
	dxv /= *index;
	dyv /= *index;
//	dx+=250;
//	dy+=250;
	node* current;
	for(i = 0; i < numNodes; i++){
		if(nodes[i].dead) continue;
		current = nodes+i;
		current->x += dx;
		current->y += dy;
		current->xmom += dxv;
		current->ymom += dyv;
	}
	return 0;
}
void taskcenteraddLong(int num, int* indices){
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskcenter;
	int* data = (int*)malloc(sizeof(int)*(num+1));
	current->dataUsed = 1;
	current->data = data;
	*data = num;
	memcpy(data+1, indices, num*sizeof(int));
	addTask(current);
}
void taskcenteradd(int i){
	taskcenteraddLong(1, &i);
}

//Not gonna bother to typedef another struct, but data is 2 ints for this guy. time is first, followed by index.
static char taskdestroy(void* where){
	int* data = (int*)where;
	if(nodes[data[1]].dead){
		free(data);
		return 1;
	}
	if(*data > 0){
		(*data)--;
		return 0;
	}else{
		killNode(data[1]);
		free(data);
		return 1;
	}
}
void taskdestroyadd(int i, int t){
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskdestroy;
	int* data = (int*)malloc(2*sizeof(int));
	current->dataUsed = 1;
	current->data = data;
	*data = t;
	data[1] = i;
	addTask(current);
}

typedef struct{
	long x;
	long y;
	int index;
	double speed;
}taskfixeddata;

static double taskfixedFactor;

static char taskfixed(void* where){
	taskfixeddata* data = (taskfixeddata*)where;
	int index = data->index;
	if(nodes[index].dead){
		free(data);
		return 1;
	}
	int deltax = data->x - nodes[index].x;
	int deltay = data->y - nodes[index].y;
	nodes[index].xmom = (nodes[index].xmom + data->speed * deltax) * taskfixedFactor;
	nodes[index].ymom = (nodes[index].ymom + data->speed * deltay) * taskfixedFactor;
	return 0;
}
void taskfixedaddLong(int i, long x, long y, double s){
	taskfixedFactor = pow(0.6, SPEEDFACTOR); // This isn't 100% efficient (as the value is calculated once per task, not once, period), but it's *good enough*
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskfixed;
	taskfixeddata* data = (taskfixeddata*)malloc(sizeof(taskfixeddata));
	current->dataUsed = 1;
	current->data = data;
	data->x = x;
	data->y = y;
	data->index = i;
	data->speed = s*SPEEDFACTOR;
	addTask(current);
}
void taskfixedadd(int i, double s){
	taskfixedaddLong(i, nodes[i].x, nodes[i].y, s);
}

typedef struct{
	int index;
	double step, max;
}taskinflatedata;

static char taskinflate(void* where){
	taskinflatedata* data = (taskinflatedata*)where;
	node* mine = nodes+data->index;
	if(mine->dead){
		free(where);
		return 1;
	}
	if(mine->size < data->max){
		mine->size += data->step;
		if(mine->size > data->max) mine->size = data->max;
	}
	return 0;
}
void taskinflateadd(int i, double step, double max){
	task* current = malloc(sizeof(task));
	current->func = &taskinflate;
	taskinflatedata* data = malloc(sizeof(taskinflatedata));
	current->dataUsed = 1;
	current->data = data;
	data->index = i;
	data->step = step;
	data->max = max;
	addTask(current);
}

static char taskfriction(void* where){
/*	register int i = 0;
	for(; i < numNodes; i++){
		if(nodes[i].dead){continue;}
		nodes[i].xmom = .985*nodes[i].xmom;
		nodes[i].ymom = .985*nodes[i].ymom;
	}*/
	return 0;
}
void taskfrictionadd(){
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskfriction;
	current->dataUsed = 0;
	addTask(current);
}

static char taskgravity(void* where){
	register int i = 0;
	for(; i < numNodes; i++){
		if(nodes[i].dead){continue;}
		nodes[i].ymom += .1*SPEEDFACTOR;
	}
	return 0;
}
void taskgravityadd(){
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskgravity;
	current->dataUsed = 0;
	addTask(current);
}

static inline void taskguycontroldisconnect(taskguycontroldata* data){
	data->controlData->inUse = 0;
	data->controltype = -1;
}
static void taskguycontroldoGun(taskguycontroldata* data){
	if(data->lastpressAction || nodes[data->controlindex].size < 1.5) return;
	int aimingLeg = -1;
	int i = 0;
	for(; i < 4; i++){
		if(data->myKeys[i]){
			if(aimingLeg == -1) aimingLeg = i;
			else{
				aimingLeg = data->connectedLeg;
				break;
			}
		}
	}
	if(aimingLeg == -1) aimingLeg = data->connectedLeg;
	if(!data->exists[(aimingLeg+2)%4]) return;
	nodes[data->controlindex].size -= 1;
	node* one = nodes+data->myNodes[aimingLeg];
	node* two = nodes+data->myNodes[(aimingLeg+2)%4];
	double dx = one->x - two->x + one->px - two->px;
	double dy = one->y - two->y + one->py - two->py;
	double dist = sqrt(dx*dx + dy*dy) / 10.0 / SPEEDFACTOR; // Velocity of the bullet
	if(dist == 0){
		dx = 0;
		dy = 0;
	}else{
		dx /= dist;
		dy /= dist;
	}
	int ix = newNodeLong(one->x, one->y, one->px, one->py, one->xmom+dx, one->ymom+dy, 2, 4, 0);
	taskdestroyadd(ix, 100);
}
static void taskguycontroldoRoll(taskguycontroldata* data){
	double rollAmt = 0;
	int* myNodes = data->myNodes;
	double rollInc = (data->myKeys[0] && (cheats&CHEAT_NUCLEAR))?10:0.03*SPEEDFACTOR;
	if(data->myKeys[3]) rollAmt += rollInc;
	if(data->myKeys[1]) rollAmt -= rollInc;
	if(!rollAmt) return;
	int i, j;
	if(rollInc > 5 && data->alive){
		data->alive = 0;
		data->injured = 1;
		for(i=0; i<4; i++){
			if(data->exists[i]) nodes[myNodes[i]].mass *= 10;
		}
	}
	node *me, *him;
	double dx, dy, dist;
	for(i=0; i < 4; i++){
		if(!data->exists[i]) continue;
		me = nodes+myNodes[i];
		for(j=me->numConnections-1; j > 0; j--){
			if(me->connections[j].dead) continue;
			him = nodes+me->connections[j].id;
			dx = him->x - me->x;
			dy = him->y - me->y;
			dist = sqrt(dx*dx+dy*dy);
			dx *= rollAmt/dist;
			dy *= rollAmt/dist;
			me->xmom -= dy;
			me->ymom += dx;
			him->xmom += dy;
			him->ymom -= dx;
		}
	}
}
static void shrinkArm(double* what, double size){
	if(*what == size) return;
	*what -= 4.0*SPEEDFACTOR;
	if(*what < size) *what = size;
}
static void taskguycontroldoLegs(taskguycontroldata* data){
	int* myNodes = data->myNodes;
	char* myKeys = data->myKeys;
//	short sl = 35;
//	short ll = 49;
	if(myKeys[5]){
		switch(data->controltype){
			case 10:
				taskguycontroldoGun(data);
				break;
			default:
				taskguycontroldoRoll(data);
				return;
		}
	}
	shrinkArm(&data->ten0, 20);
	shrinkArm(&data->ten1, 20);
	shrinkArm(&data->nine0, 20);
	shrinkArm(&data->nine1, 20);
	shrinkArm(&data->nine2, 28);
	shrinkArm(&data->eleven0, 28);
	short sl = 0, ll = 0;
	int count = 0;
	int i = 0;
	for(; i < 4; i++){
		if(myKeys[i]) count++;
	}
	if(count != 0){
		if(count == 3) count = 4;
		sl = 20+20/count;
		ll = 28+28/count;
	}
	if(myKeys[0]){
		data->nine0 = sl;
		data->nine1 = sl;
		data->nine2 = ll;
	}
	if(myKeys[2]){
		data->ten0 = sl;
		data->ten1 = sl;
		data->nine2 = ll;
	}
	if(myKeys[3]){
		data->ten0 = sl;
		data->nine0 = sl;
		data->eleven0 = ll;
	}
	if(myKeys[1]){
		data->ten1 = sl;
		data->nine1 = sl;
		data->eleven0 = ll;
	}
	if(data->exists[0]){
		node* current = nodes + myNodes[0];
		if(!current->connections[2].dead)
			current->connections[2].preflength = data->nine1;
		if(!current->connections[3].dead)
			current->connections[3].preflength = data->nine2;
		if(!current->connections[1].dead)
			current->connections[1].preflength = data->nine0;
	}
	if(data->exists[2]){
		node* current = nodes + myNodes[2];
		if(!current->connections[1].dead)
			current->connections[1].preflength = data->ten0;
		if(!current->connections[2].dead)
			current->connections[2].preflength = data->ten1;
	}
	if(data->exists[3]){
		node* current = nodes + myNodes[3];
		if(!current->connections[1].dead)
			current->connections[1].preflength = data->eleven0;
	}
}
static void toolGravity(){
	int i = 0;
	for(; i < numNodes; i++){
		if(nodes[i].dead) continue;
		nodes[i].ymom -= .1*SPEEDFACTOR;
	}
}
static void toolBigLegs(taskguycontroldata* data){
	char* myKeys = data->myKeys;
	int centerDists[4] = {42, 42, 42, 42};
	int edgeLengths[4] = {60, 60, 60, 60};
	int i = 0;
	if(myKeys[5]){
		centerDists[0] = centerDists[1] = centerDists[2] = centerDists[3] = 74;
		edgeLengths[0] = edgeLengths[1] = edgeLengths[2] = edgeLengths[3] = 105;
	}else{
		for(; i < 4; i++){
			if(myKeys[i]){
				centerDists[i] = 74;
				edgeLengths[i] = 105;
				edgeLengths[(i+1)%4] = 105;
			}
		}
	}
	int controlindex = data->controlindex;
	node* center = nodes+controlindex;
	for(i = 0; i < 4; i++){
		if(!center->connections[i].dead){
			center->connections[i].preflength = centerDists[i];
			if(!nodes[controlindex+i+1].connections[0].dead)
				nodes[controlindex+i+1].connections[0].preflength = edgeLengths[i];
		}
	}
	if(nodes[controlindex+1].dead) return;
	int size = (int)(maxZoomIn*5);
	int x = nodes[controlindex+1].x*maxZoomIn;
	int y = nodes[controlindex+1].y*maxZoomIn;
	if(frameCount == 0){
		if(netMode) addNetCircle(x, y, size);
		setColorWhite();
		drawCircle(getScreenX(x-centerx), getScreenY(y-centery), (double)size/zoom/width2);
	}
}
static int taskguycontrolcreateBody(taskguycontroldata* data){
	int x = data->respawnx-10;
	int y = data->respawny-10;
	int i = newNode(x, y, 6, 2, 4);
	int i1= newNode(x+20, y   , 6, 2, 1);
	int i2= newNode(x+20, y+20, 6, 2, 3);
	int i3= newNode(x+0 , y+20, 6, 2, 2);
	nodes[i ].connections[0].dead = 1;
	nodes[i1].connections[0].dead = 1;
	nodes[i2].connections[0].dead = 1;
	nodes[i3].connections[0].dead = 1;
	newConnectionLong(i2, 1, i3, 0.7, 20, 35, 23, .35);
	newConnectionLong(i2, 2, i1, 0.7, 20, 35, 23, .35);
	newConnectionLong(i,   1, i3, 0.7, 20, 35, 23, .35);
	newConnectionLong(i,   2, i1, 0.7, 20, 35, 23, .35);
	newConnectionLong(i,   3, i2, 0.7, 28, 49, 32.2, .35);
	newConnectionLong(i3, 1, i1, 0.7, 28, 49, 32.2, .35);

	data->myNodes[0] = i;
	data->myNodes[1] = i1;
	data->myNodes[2] = i2;
	data->myNodes[3] = i3;
	data->alive = 1;
	data->injured = 0;
	data->ten0 = data->ten1 = data->nine0 = data->nine1 = 20;
	data->nine2 = data->eleven0 = 28;
	int ix = 0;
	for(; ix < 4; ix++) data->exists[ix] = 1;
	return i;
}
static char taskguycontrol(void* where){
	taskguycontroldata* data = (taskguycontroldata*)where;
	int* myNodes = data->myNodes;
	if(data->alive){
		int counter = 0;
		data->centerX = data->centerY = 0;
		int i = 0;
		int j;
		for(; i < 4; i++){
			if(!data->exists[i]) continue;
			node* current = nodes + myNodes[i];
			if(current->dead){
				if(data->controltype != -1 && data->connectedLeg==i) taskguycontroldisconnect(data);
				data->exists[i] = 0;
				data->injured = 1;
			}else{
				counter++;
				data->centerX += current->x;
				data->centerY += current->y;
			}
			if(data->injured){
				for(j = current->numConnections-1; j >= 1; j--){
					if(current->connections[j].dead){
						data->injured = 1;
						break;
					}
				}
			}
		}
		if(counter == 0){
			data->alive = 0;
		}else{
			data->centerX /= counter;
			data->centerY /= counter;
		}
	}
	char* myKeys = data->myKeys;
	if(!data->lastpress&&myKeys[4]){
		if(data->controltype != -1){
			nodes[myNodes[data->connectedLeg]].connections[0].dead = 1;
			taskguycontroldisconnect(data);
		}else{
			int min = 0;//I feel kinda bad doing this, as I'm only trying to stop the appearance of a warning about uninitialized variables... I do, though, have the uninitialized variable situation under control. Not to worry.
			int current;
			int deltax;
			int deltay;
			data->controlData = NULL;
			int leg = 0;
			int i;
			for(; leg < 4; leg++){
				if(!data->exists[leg]) continue;
				int mx = (int)nodes[myNodes[leg]].x;
				int my = (int)nodes[myNodes[leg]].y;
				for(i = 0; i < numTools; i++){
					if(tools[i].where==-1 || tools[i].inUse || nodes[tools[i].where].dead) continue;
					deltax = (int)(mx - nodes[tools[i].where].x);
					deltay = (int)(my - nodes[tools[i].where].y);
					current = (int)sqrt(deltax*deltax + deltay*deltay);
					if(current < nodes[tools[i].where].size+18 && (current < min || data->controlData == NULL)){
						min = current;
						data->controlData = tools+i;
						data->connectedLeg = leg;
					}
				}
			}
			if(data->controlData != NULL){
				data->controltype = data->controlData->type;
				data->controlindex = data->controlData->where;
				newConnection(myNodes[data->connectedLeg], 0, data->controlindex, (double)0.8, (int)nodes[data->controlindex].size+8, 10, 0.8);
				data->controlData->inUse = 1;
				if(data->controltype == 0){
						killNode(data->controlindex);
						taskguycontroldisconnect(data);
				}else if(data->controltype == 2){
					connection* con = nodes[data->controlindex].connections;
					if(!con->dead) con->preflength -= 2*(con->preflength - con->midlength);
					taskguycontroldisconnect(data);
					nodes[myNodes[data->connectedLeg]].connections[0].dead = 1;
				}
			}
		}
	}
	else if(data->controltype != -1 && (nodes[myNodes[data->connectedLeg]].connections[0].dead || nodes[data->controlindex].dead)){taskguycontroldisconnect(data);}
	switch(data->controltype){
	case 100:
		toolBigLegs(data);
		break;
	case 70:
		toolGravity();
		taskguycontroldoLegs(data);
	default:
		taskguycontroldoLegs(data); // Also handles some tools (gun)
		break;
	}
	if(frameCount == 0 && data->exists[0]){
		if(netMode)
			addNetPlayerCircle(nodes[myNodes[0]].netIndex, data->respawnx*maxZoomIn, data->respawny*maxZoomIn, requests[data->num].color);
		setColorFromHex(requests[data->num].color);
		drawCircle(getScreenX(nodes[myNodes[0]].x*maxZoomIn-centerx), getScreenY(nodes[myNodes[0]].y*maxZoomIn-centery), (float)markSize/2/width2);
		drawCircle(getScreenX(data->respawnx*maxZoomIn-centerx), getScreenY(data->respawny*maxZoomIn-centery), (float)markSize/2/width2);
	}

	data->lastpressAction = myKeys[5];
	if(myKeys[4]){
		data->lastpress++;
		if(data->lastpress == (int)(30/SPEEDFACTOR)){
			if(myKeys[5]){
				if(data->controltype!=-1) taskguycontroldisconnect(data);
				taskguycontrolcreateBody(data);
			}else{
				data->respawnx=data->centerX;
				data->respawny=data->centerY;
			}
		}
	}else
		data->lastpress = 0;
	return 0;
}
void taskguycontroladd(int x, int y){
	task* current = (task*)malloc(sizeof(task));
	addTask(current);

	current->func = &taskguycontrol;
	taskguycontroldata* data = guyDatas+playerNum;
	data->respawnx = data->centerX = x+10;
	data->respawny = data->centerY = y+10;
	data->num = playerNum;
	taskguycontrolcreateBody(data);
	current->dataUsed = 0;//Set to 0 so it isn't free'd when the task exits.
	current->data = data;
	data->myKeys = masterKeys+NUMKEYS*playerNum;
	data->controltype = -1;
	data->lastpress = 0;

	int ix, controlMode = requests[playerNum].controlMode;
	if(controlMode>=2 && controlMode<=4){
		int* myNodes = data->myNodes;
		taskaicombatadd(playerNum, controlMode==4);
		if(controlMode==2){
			int j;
			node* current;
			for(ix=0; ix<4; ix++){
				current = nodes + myNodes[ix];
				current->mass /= 2;
				for(j=1; j<current->numConnections; j++) current->connections[j].force /= 2;
			}
		}
	}
	else if(controlMode==5 || controlMode==6) taskaispacecombatadd(playerNum, controlMode==6);
	playerNum++;
}
uint32_t getToolColor(int type){
	switch(type){
		case 0:
			return 0xFFFF00FF;
		case 1:
			return 0x000000FF;
		case 2:
			return 0x00FF00FF;
		case 10:
			return 0xFF0000FF;
		case 70:
			return 0x5000FFFF;
		case 100:
			return 0xC04040FF;
		default:
			return 0xFF00FFFF;
	}
}
void addGenericTool(int ix, int type){
	int i = addTool();
	tools[i].type = type;
	tools[i].where = ix;
	tools[i].inUse = 0;
}
void addToolMech1(int x, int y){
	int ix = newNode(x, y, 6, 18, 4);
	addGenericTool(ix, 100);
	newNode(x-30, y-30, 10, 18, 1);
	newNode(x+30, y-30, 10, 18, 1);
	newNode(x+30, y+30, 10, 18, 1);
	newNode(x-30, y+30, 10, 18, 1);
	int i = 1;
	for(; i < 5; i++){
		newConnectionLong(ix, i-1, ix+i, .6, i>2?74:42, 58, 34, 1.05);
		newConnectionLong(ix+i, 0, ix+(i+2)%4+1, .6, i==2?60:105, 82, 45, 1.05);
	}
}
void addToolGun(double x, double y){
	int ix = newNode(x, y, 3, 1, 0);
	addGenericTool(ix, 10);
	taskinflateadd(ix, .1, 3);
}
void addToolDestroy(int ix){
	addGenericTool(ix, 0);
}
void addToolGrab(int ix){
	addGenericTool(ix, 1);
}
void addToolToggle(int ix){
	addGenericTool(ix, 2);
}
void addToolGravity(int ix){
	addGenericTool(ix, 70);
}

static char taskincinerator(void* where){
	register int i = 0;
	long l = *(long*)where;
	for(; i < numNodes; i++){
		if(nodes[i].dead){continue;}
		if(nodes[i].y - nodes[i].size > l) killNode(i);
	}
	return 0;
}

void taskincineratoradd(long height){
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskincinerator;
	current->data = malloc(sizeof(long));
	current->dataUsed = 1;
	*(long*)current->data = height;
	addTask(current);
}

static char taskincinerator2(void* where){
	register int i = 0;
	long height = *(long*)where;
	for(; i < numNodes; i++){
		if(nodes[i].dead){continue;}
		if(fabs(nodes[i].y) - nodes[i].size > height ||
		   fabs(nodes[i].x) - nodes[i].size > height)     killNode(i);
	}
	return 0;
}
void taskincinerator2add(long height){
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskincinerator2;
	current->dataUsed = 1;
	current->data = malloc(sizeof(long));
	*(long*)current->data = height;
	addTask(current);
}

typedef struct{
	int ix;
	double gravity;
} taskpointgravitydata;

static char taskpointgravity(void* where){
	taskpointgravitydata* data = (taskpointgravitydata*)where;
	if(nodes[data->ix].dead){
		free(data);
		return 1;
	}
	long x = nodes[data->ix].x;
	long y = nodes[data->ix].y;
	int dx, dy;
	double dist, force;
	register int i = 0;
	double gravity = data->gravity;
	for(; i < numNodes; i++){
		if(nodes[i].dead || i==data->ix) continue;
		dx = (int)(x - nodes[i].x);
		dy = (int)(y - nodes[i].y);
		dist = dx*dx + dy*dy;
		force = gravity / dist;
		dist = sqrt(dist);
		nodes[i].xmom += force*dx/dist;
		nodes[i].ymom += force*dy/dist;
	}
	return 0;
}
void taskpointgravityadd(int ix, double gravity){
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskpointgravity;
	taskpointgravitydata* data = malloc(sizeof(taskpointgravitydata));
	data->ix = ix;
	data->gravity = gravity*SPEEDFACTOR;
	current->dataUsed = 1;
	current->data = data;
	addTask(current);
}
static char taskuniversalgravity(void* where){
	double G = *(double*)where;
	register int i = 0;
	register int j = 0;
	long x, y;
	int dx, dy;
	double dist, force, gravity;
	for(; i < numNodes; i++){
		if(nodes[i].dead) continue;
		x = nodes[i].x;
		y = nodes[i].y;
		gravity = nodes[i].mass*G;
		for(j = i+1; j < numNodes; j++){
			if(nodes[j].dead) continue;
			dx = x - nodes[j].x;
			dy = y - nodes[j].y;
			if(dx==0 && dy==0) continue;
			dist = dx*dx + dy*dy;
			force = gravity / dist;
//This task treats gravity as being inversely proportional to the distance (not its square).
//This is in part due to the fact that the game is two dimentional, and in the author's
//belief is justified in part by the fact that radiant intensity is also proportional to the
//distance in a two dimensional world. The above line, by dividing by the square of the
//ditance, calculates force per unit offset. This means the next lines act as they should.
			nodes[j].xmom += force*dx;
			nodes[j].ymom += force*dy;
			force = -nodes[j].mass * G / dist;
			nodes[i].xmom += force*dx;
			nodes[i].ymom += force*dy;
		}
	}
	return 0;
}
void taskuniversalgravityadd(double gravity){
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskuniversalgravity;
	double* data = malloc(sizeof(double));
	*data = gravity*SPEEDFACTOR;
	current->data = data;
	current->dataUsed = 1;
	addTask(current);
}

typedef struct{
	int index;
	int score;
	int x, y;
	char done;
} taskscoredata;

static char taskscore(void* where){
	taskscoredata* data = (taskscoredata*)where;
	if(!data->done){
		if(guyDatas[data->index].injured) data->done = 1;
		else{
			data->score++;
			if(data->score == 999999) data->done = 1;//So we don't overflow our char*
		}
	}
	if(frameCount) return 0;
	char* text = malloc(sizeof(char)*7);
	sprintf(text, "%d", data->score);
	setColorWhite();
	drawText(data->x, data->y, 1.5, text);
	free(text);
	return 0;
}
void taskscoreaddLong(int ix, int x, int y){
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskscore;
	taskscoredata* data = malloc(sizeof(taskscoredata));
	data->index = ix;
	data->score = 0;
	data->x = x-width2;
	data->y = y+height2;
	data->done = 0;
	current->dataUsed = 1;
	current->data = data;
	addTask(current);
}
void taskscoreadd(int ix){
	taskscoreaddLong(ix, 8, 20);
}

typedef struct{
	int x, y;
	char* text;
} tasktextdata;

static char tasktext(void* where){
	if(frameCount) return 0;
	tasktextdata* data = (tasktextdata*)where;
	setColorWhite();
	drawText((data->x*maxZoomIn - centerx)/zoom, (data->y*maxZoomIn - centery)/zoom, 1.0*maxZoomIn/zoom, data->text);
	return 0;
}

void tasktextadd(int x, int y, char* text){
	task* current = (task*)malloc(sizeof(task));
	current->func = &tasktext;
	tasktextdata* data = malloc(sizeof(tasktextdata));
	data->x = x; data->y = y; data->text = text;
	current->dataUsed = 1;
	current->data = data;
	addTask(current);
}
