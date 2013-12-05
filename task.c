#include <stdlib.h>
#include "structs.h"
#include "gui.h"
#include "field.h"
#include "node.h"
#include "font.h"
#include "gfx.h"
#include "networking.h"
#include <math.h>

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
	int index;
	Sint8 mode;
	int cycle;
	Sint8* myKeys;
} taskaicombatdata;

Sint8 taskaicombat(void* where){
	taskaicombatdata* data = (taskaicombatdata*)where;
	if(nodes[data->index].dead){
		free(data);
		return 1;
	}
	if(data->cycle-- == 0){
		data->cycle = 10;
		data->mode = !data->mode;
		int i = 0;
		for(; i < NUMKEYS; i++) data->myKeys[i]=0;
		if(data->mode){
			int index = data->index;
			if(injured[data->target]){
				int targets[players];
				int numTargets = 0;
				int i = 0;
				for(; i < players; i++){
					if(!injured[i] && i != data->player)
						targets[numTargets++] = i;
				}
				if(numTargets == 0) return 1;//We're the last one standing
				data->target = targets[(long int)rand()*numTargets/RAND_MAX];
			}
			int target = taskguycontrolindexes[data->target];
			Sint8 dir = nodes[target].x > nodes[index].x;
			long maxHeight = 0;
			int ix = -1;
			int i = 0;
			for(; i < 4; i++){
				if(nodes[index+i].dead) continue;
				long fitness = nodes[index+i].y+(dir?-nodes[index+i].x:nodes[index+i].x);
				if(fitness > maxHeight || ix == -1){
					ix = i;
					maxHeight = fitness;
				}
			}
			data->myKeys[ix] = 1;
		}
	}
	return 0;
}
void taskaicombatadd(int Player){
	if(players < 2) return;
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskaicombat;
	taskaicombatdata* data = (taskaicombatdata*)malloc(sizeof(taskaicombatdata));
	current->dataUsed = 1;
	current->data = data;
	int target = (long)rand() * (players-1) / RAND_MAX;
	if(target >= Player) target++;
	data->target = target;
	data->cycle = 5;
	data->player = Player;
	data->index = taskguycontrolindexes[Player];
	data->mode = 1;
	data->myKeys = masterKeys + NUMKEYS*Player;
	addTask(current);
}

typedef struct {
	int size;
	int mass;
	int cool;
	int maxcool;
} taskasteroidsdata;
Sint8 taskasteroids(void* where){
	taskasteroidsdata* data = (taskasteroidsdata*)where;
	if(data->cool-- <= 0){
		newNodeLong(addNode(), (float)rand()/RAND_MAX*400, 0, 0, 0, (float)rand()/RAND_MAX*4-2, 0, data->size, data->mass, 0);
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
	data->maxcool = c;
	data->cool = c;
	addTask(current);
}

Sint8 taskcenter(void* where){
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
	dx+=250;
	dy+=250;
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
Sint8 taskdestroy(void* where){
	int* data = (int*)where;
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
Sint8 taskfixed(void* where){
	taskfixeddata* data = (taskfixeddata*)where;
	int index = data->index;
	if(nodes[index].dead){
		free(data);
		return 1;
	}
	int deltax = data->x - nodes[index].x;
	int deltay = data->y - nodes[index].y;
	nodes[index].xmom = (nodes[index].xmom + data->speed * deltax) * .6;
	nodes[index].ymom = (nodes[index].ymom + data->speed * deltay) * .6;
	return 0;
}
void taskfixedaddLong(int i, long x, long y, double s){
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskfixed;
	taskfixeddata* data = (taskfixeddata*)malloc(sizeof(taskfixeddata));
	current->dataUsed = 1;
	current->data = data;
	data->x = x;
	data->y = y;
	data->index = i;
	data->speed = s;
	addTask(current);
}
void taskfixedadd(int i, double s){
	taskfixedaddLong(i, nodes[i].x, nodes[i].y, s);
}

Sint8 taskfriction(void* where){
	register int i = 0;
	for(; i < numNodes; i++){
		if(nodes[i].dead){continue;}
		nodes[i].xmom = .985*nodes[i].xmom;
		nodes[i].ymom = .985*nodes[i].ymom;
	}
	return 0;
}
void taskfrictionadd(){
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskfriction;
	current->dataUsed = 0;
	addTask(current);
}

Sint8 taskgravity(void* where){
	register int i = 0;
	for(; i < numNodes; i++){
		if(nodes[i].dead){continue;}
		nodes[i].ymom += .1;
	}
	return 0;
}
void taskgravityadd(){
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskgravity;
	current->dataUsed = 0;
	addTask(current);
}

typedef struct{
	int index;
	int controltype;
	int controlindex;
	//int controlvar;//Currently unused, but intended as a multi-purpose variable for the current tool.
	Sint8 lastpress;
	Sint8* myKeys;
	int num;
	tool* controlData;
	int connectedLeg;
	Sint8 exists[4];
}taskguycontroldata;

inline void taskguycontroldisconnect(taskguycontroldata* data){
	data->controlData->type = data->controltype;
	data->controltype = -1;
}
static void taskguycontroldoLegs(taskguycontroldata* data){
	int index = data->index;
	Sint8* myKeys = data->myKeys;
	short ten0 = 20;
	short ten1 = 20;
	short nine0 = 20;
	short nine1 = 20;
	short nine2 = 28;
	short eleven0 = 28;
	short sl = 35;
	short ll = 49;
	if(myKeys[0]){
		nine0 = sl;
		nine1 = sl;
		nine2 = ll;
	}
	if(myKeys[2]){
		ten0 = sl;
		ten1 = sl;
		nine2 = ll;
	}
	if(myKeys[3]){
		ten0 = sl;
		nine0 = sl;
		eleven0 = ll;
	}
	if(myKeys[1]){
		ten1 = sl;
		nine1 = sl;
		eleven0 = ll;
	}
	if(!nodes[index].connections[1].dead){
		nodes[index].connections[1].preflength = nine0;
	}
	if(!nodes[index].connections[2].dead){
		nodes[index].connections[2].preflength = nine1;
	}
	if(!nodes[index].connections[3].dead){
		nodes[index].connections[3].preflength = nine2;
	}
	if(data->exists[2]){
		if(!nodes[index+2].connections[1].dead){
			nodes[index+2].connections[1].preflength = ten0;
		}
		if(!nodes[index+2].connections[2].dead){
			nodes[index+2].connections[2].preflength = ten1;
		}
	}
	if(data->exists[3]){
		if(!nodes[index+3].connections[1].dead){
			nodes[index+3].connections[1].preflength = eleven0;
		}
	}
}
inline void taskguycontroldoBigLegs(taskguycontroldata* data){
	Sint8* myKeys = data->myKeys;
	int centerDists[4] = {42, 42, 42, 42};
	int edgeLengths[4] = {60, 60, 60, 60};
	int i = 0;
	for(; i < 4; i++){
		if(myKeys[i]){
			centerDists[i] = 74;
			edgeLengths[i] = 105;
			edgeLengths[(i+1)%4] = 105;
		}
	}
	int controlindex = data->controlindex;
	node* center = nodes+controlindex;
	for(i = 0; i < 4; i++){
		if(!center->connections[i].dead)
			center->connections[i].preflength = centerDists[i];
		if(!nodes[controlindex+i+1].connections[0].dead)
			nodes[controlindex+i+1].connections[0].preflength = edgeLengths[i];
	}
	if(nodes[controlindex+1].dead) return;
	int size = (int)(maxZoomIn*5);
	int x = nodes[controlindex+1].x*maxZoomIn;
	int y = nodes[controlindex+1].y*maxZoomIn;
	if(netMode) addNetCircle(x, y, size);
	size /= zoom;
	if(size < 1) size = 1;
	ellipseColor(screen, getScreenX(x-centerx), getScreenY(y-centery), size, size, 0xFFFFFFFF);
}
Sint8 taskguycontrol(void* where){
	taskguycontroldata* data = (taskguycontroldata*)where;
	int num = data->num;
	int index = data->index;
	if(nodes[index].dead){
		if(data->controltype!=-1) data->controlData->type = data->controltype;
		alives[num] = 0;
		injured[num] = 1;
		free(data);
		return 1;
	}
	{
		alives[num] = 1;
		int counter = 0;
		double myCenters[2] = {0, 0};
		int i = 0;
		int j;
		for(; i < 4; i++){
			if(!data->exists[i]) continue;
			if(nodes[index+i].dead){
				data->exists[i] = 0;
			}else{
				counter++;
				myCenters[0] += nodes[index+i].x;
				myCenters[1] += nodes[index+i].y;
			}
			if(!injured[num]){
				for(j = nodes[index+i].numConnections-1; j >= 1; j--){
					if(nodes[index+i].connections[j].dead){
						injured[num] = 1;
						break;
					}
				}
			}
		}
		centers[num].x = myCenters[0]/counter;
		centers[num].y = myCenters[1]/counter;
	}
	Sint8* myKeys = data->myKeys;
	if(!data->lastpress&&myKeys[4]){
		if(data->controltype != -1){
			nodes[index+data->connectedLeg].connections[0].dead = 1;
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
				int mx = (int)nodes[index+leg].x;
				int my = (int)nodes[index+leg].y;
				for(i = 0; i < numTools; i++){
					if(tools[i].where==-1 || tools[i].type==-1 || nodes[tools[i].where].dead) continue;
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
				newConnection(index+data->connectedLeg, 0, data->controlindex, (double)0.8, (int)nodes[data->controlindex].size+8, 10, 0.8);
				data->controlData->type = -1;
				switch(data->controltype){
					//Things to do on connect
					case 0:
						killNode(data->controlindex);
						taskguycontroldisconnect(data);
						break;
					default:
						break;
				}
			}
		}
	}
	else if(data->controltype != -1 && (nodes[index+data->connectedLeg].connections[0].dead || nodes[data->controlindex].dead)){taskguycontroldisconnect(data);}
	data->lastpress = myKeys[4];
	switch(data->controltype){
	case -1:
		taskguycontroldoLegs(data);
		break;
	case 100:
		taskguycontroldoBigLegs(data);
		break;
	}
	if(netMode)
		addNetPlayerCircle(index, requests[data->num].color);
	ellipseColor(screen, getScreenX(nodes[index].x*maxZoomIn-centerx), getScreenY(nodes[index].y*maxZoomIn-centery), markSize/2, markSize/2, requests[data->num].color);
	return 0;
}
void taskguycontroladdLong(int x, int y, Sint8 flipped){
	int i = addNode();
	newNode(i, x, y, 6, 2, 4);
	task* current = (task*)malloc(sizeof(task));
	addTask(current);
	taskguycontrolindexes[playerNum] = i;
	centers[playerNum].x = x+(flipped?-10:10);
	centers[playerNum].y = y+10;
	alives[playerNum] = 1;

	current->func = &taskguycontrol;
	taskguycontroldata* data = (taskguycontroldata*)malloc(sizeof(taskguycontroldata));
	current->dataUsed = 1;
	current->data = data;
	data->myKeys = masterKeys+NUMKEYS*playerNum;
	if(requests[playerNum].controlMode == 2) taskaicombatadd(playerNum);
	data->index = i;
	data->num = playerNum++;
	data->controltype = -1;
	int ix = 0;
	for(; ix < 4; ix++) data->exists[ix] = 1;
	data->lastpress = 0;

	newNode(addNode(), x+(flipped?0:20), y+(flipped?20:0), 6, 2, 1);
	newNode(addNode(), x+(flipped?-20:20), y+20, 6, 2, 3);
	newNode(addNode(), x+(flipped?-20:0), y+(flipped?0:20), 6, 2, 2);
	nodes[ i ].connections[0].dead = 1;
	nodes[i+1].connections[0].dead = 1;
	nodes[i+2].connections[0].dead = 1;
	nodes[i+3].connections[0].dead = 1;
	newConnectionLong(i+2, 1, i+3, 0.6, 20, 27, 15, .35);
	newConnectionLong(i+2, 2, i+1, 0.6, 20, 27, 15, .35);
	newConnectionLong(i,   1, i+3, 0.6, 20, 27, 15, .35);
	newConnectionLong(i,   2, i+1, 0.6, 20, 27, 15, .35);
	newConnectionLong(i,   3, i+2, 0.6, 28, 39, 19, .35);
	newConnectionLong(i+3, 1, i+1, 0.6, 28, 39, 19, .35);
}
void taskguycontroladd(int x, int y){taskguycontroladdLong(x, y, 0);}
void taskguycontroladdToolMech1(int x, int y){
	int ix = addNode();
	newNode(ix, x, y, 6, 18, 4);
	int tix = addTool();
	tools[tix].type = 100;
	tools[tix].where = ix;
	newNode(addNode(), x-30, y-30, 10, 18, 1);
	newNode(addNode(), x+30, y-30, 10, 18, 1);
	newNode(addNode(), x+30, y+30, 10, 18, 1);
	newNode(addNode(), x-30, y+30, 10, 18, 1);
	int i = 1;
	for(; i < 5; i++){
		newConnectionLong(ix, i-1, ix+i, .6, i>2?74:42, 58, 34, 1.05);
		newConnectionLong(ix+i, 0, ix+(i+2)%4+1, .6, i==2?60:105, 82, 45, 1.05);
	}
}
void taskguycontroladdToolDestroy(int ix){
	int i = addTool();
	tools[i].type = 0;
	tools[i].where = ix;
}

Sint8 taskincinerator(void* where){
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

Sint8 taskincinerator2(void* where){
	register int i = 0;
	long height = *(long*)where;
	for(; i < numNodes; i++){
		if(nodes[i].dead){continue;}
		if(fabs(nodes[i].y-250) - nodes[i].size > height ||
		   fabs(nodes[i].x-250) - nodes[i].size > height)     killNode(i);
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

Sint8 taskpointgravity(void* where){
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
	data->gravity = gravity;
	current->dataUsed = 1;
	current->data = data;
	addTask(current);
}
Sint8 taskuniversalgravity(void* where){
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
//distance in a two dimensional world. The above line, by dividing the the square of the
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
	*data = gravity;
	current->data = data;
	current->dataUsed = 1;
	addTask(current);
}

typedef struct{
	int index;
	int score;
	int x, y;
	Sint8 done;
} taskscoredata;

Sint8 taskscore(void* where){
	taskscoredata* data = (taskscoredata*)where;
	if(!data->done){
		if(injured[data->index]) data->done = 1;
		else{
			data->score++;
			if(data->score == 999999) data->done = 1;//So we don't overflow our char*
		}
	}
	char* text = malloc(sizeof(char)*7);
	sprintf(text, "%d", data->score);
	drawText(screen, data->x, data->y, 0xFFFFFFFF, 1.5, text);
	free(text);
	return 0;
}
void taskscoreaddLong(int ix, int x, int y){
	task* current = (task*)malloc(sizeof(task));
	current->func = &taskscore;
	taskscoredata* data = malloc(sizeof(taskscoredata));
	data->index = ix;
	data->score = 0;
	data->x = x;
	data->y = y;
	data->done = 0;
	current->dataUsed = 1;
	current->data = data;
	addTask(current);
}
void taskscoreadd(int ix){
	taskscoreaddLong(ix, 8, 20);
}
