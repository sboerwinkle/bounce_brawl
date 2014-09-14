#include "structs.h"
#include "field.h"
#include "gui.h"

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

int achieveFlood(){
	int i = 0;
	for(; i < players; i++){
		if(guyDatas[i].controlType == 70) return 1;
	}
	return 0;
}
