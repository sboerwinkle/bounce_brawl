#include "structs.h"
#include "field.h"
#include "gui.h"

int achieveLazy(){
	return 2;
}

int achieveFlawless(){
	int i = 0;
	for(; i < players; i++){
		if(guyDatas[i].injured || !guyDatas[i].firstLife) return 1;
	}
	return 2;
}
