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
