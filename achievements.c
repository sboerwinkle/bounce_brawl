#include "structs.h"
#include "field.h"
#include "gui.h"
#include "math.h"

int achieveLazy(){
	return 1;
}

int achieveFlawless(){
	int i = 0;
	for(; i < players; i++){
		if(guyDatas[i].injured || !guyDatas[i].firstLife) return 1;
	}
	return 2;
}

int achieveGardens(){
	int i = 0;
	for(; i < players; i++){
		if(guyDatas[i].controlType == 70) return 1;
	}
	return 0;
}

int achieveCave(){
	int i = 0;
	for(; i < players; i++){
		if(abs(guyDatas[i].centerX)<50 && guyDatas[i].centerY<-50) return 1;
	}
	return 0;
}

int achieveBuilding(){
	int i = 0;
	for(; i < players; i++){
		if(guyDatas[i].centerY<-325) return 1;
	}
	return 0;
}

static int achieveDestruction(numAcceptable){
	int alive = -4*players-numAcceptable;
	int i = 0;
	for(; i < numNodes; i++){
		if(!nodes[i].dead)
			if(0==alive++) return 0;
	}
	return 1;
}

int achieveSumo(){return achieveDestruction(70);}

int achieveDrop(){return achieveDestruction(20);}

int achieveWalled(){return achieveDestruction(20);}

int achieveBoulder(){
	int i = 0;
	for(; i < numNodes; i++){
		if(!nodes[i].dead && 3==nodes[i].mass) return 0;
	}
	return 1;
}

int achievePlain(){
	int i = 0;
	for(; i < numNodes; i++){
		if(!nodes[i].dead && 1==nodes[i].mass) return 0;
	}
	return 1;
}

int achieveGunMech(){
	int i = 0;
	for(; i < numNodes; i++){
		if(!nodes[i].dead && (18==nodes[i].mass || 1==nodes[i].mass)) return 0;
	}
	return 1;
}

int achieveAsteroids(){
	int i = 0, j;
	for(; i < numNodes; i++){
		if(nodes[i].dead || nodes[i].size!=10) continue;
		for(j=0; j<players; j++){
			double dx = guyDatas[j].centerX-nodes[i].x;
			double dy = guyDatas[j].centerY-nodes[i].y;
			if(dx*dx+dy*dy <= 100) return 1;
		}
	}
	return 0;
}

int achievePlanet(){
	int i = 0;
	for(; i < players; i++){
		double x = guyDatas[i].centerX;
		double y = guyDatas[i].centerY;
		if(sqrt(x*x + y*y) > 390) return 1;
	}
	return 0;
}
