#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
//#include <SDL2/SDL2_gfxPrimitives.h>
#include "structs.h"
#include "font.h"
#include "levels.h"
#include "field.h"
#include "networking.h"

#ifdef WINDOWS
#include <windows.h>
#endif

#define NUMKEYS 5
#define TEXTSIZE 2
//If these are changed, change gui.h

typedef struct{
	int highscore;
	void (*func)();
} level;

typedef struct{
	Sint8 menu;
	void* target;
	char* text;
} menuItem;

struct menu{
	struct menu* parent;
	int numItems;
	menuItem* items;
};
typedef struct menu menu;

menu* addMenuMenu(menu* parent, int ix, int numItems, char* text){
	menu* ret = malloc(sizeof(menu));
	ret->parent = parent;
	ret->numItems = numItems;
	ret->items = malloc(numItems*sizeof(menuItem));
	menuItem* item = parent->items+ix;
	item->menu = 1;
	item->target = ret;
	item->text = text;
	return ret;
}

void addMenuLevel(menu* where, int ix, void (*func)(), char* text){
	menuItem* item = where->items+ix;
	item->menu = 0;
	item->target = malloc(sizeof(level));
	((level*)item->target)->func = func;
	((level*)item->target)->highscore = 0;
	item->text = text;
}

menu* currentMenu;

static SDL_Window* window;
static SDL_Renderer* render;
static SDL_Texture* texture;
Uint32 *screen;

int players = 0;
static int numRequests;
playerRequest requests[10];

Sint8 masterKeys[NUMKEYS*10];
int pIndex[] = {1, 0};

Sint8 mode = 0;
static Sint8 inputMode = 0;
static Sint8 nothingChanged = 0;
static Sint8 sloMo = 0;
static char frameFlag = 0;

void myDrawScreen(){
	if(SDL_UpdateTexture(texture, NULL, screen, 750*4) < 0) puts(SDL_GetError());
	if(SDL_RenderClear(render) < 0) puts(SDL_GetError());
	if(SDL_RenderCopy(render, texture, NULL, NULL) < 0) puts(SDL_GetError());
	SDL_RenderPresent(render);
}

static char* modeToString(int ix){
	switch(requests[ix].controlMode){
		case -1:
			return "NETWORKED PLAYER";
		case 0:
			return "ARROW KEYS + RIGHT CONTROL";
		case 1:
			return "WASD + Q";
		case 2:
			return "COMBAT AI";
		default:
			return "Error! Danger! Augh!";
	}
}

static void paint(){
	if(mode == 0){
		if(nothingChanged) return;
//		boxColor(render, 0, 0, 500, 500, 0x000000FF);
		memset(screen, 0, 750*750*4);
//		SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
//		SDL_Rect a = {.x=0, .y=0, .w=500, .h=500};
//		SDL_RenderFillRect(render, &a);
		char* line = malloc(40*sizeof(char));
		if(inputMode == 0){
			int i = 0;
			for(; i < currentMenu->numItems; i++){
				sprintf(line, " %d : %s", i+1, currentMenu->items[i].text);
				drawText(screen, 20, 20+9*TEXTSIZE*i, 0xFFFFFFFF, TEXTSIZE, line);
			}
//			drawText(render, 20, 20, 0xFFFFFFFF, TEXTSIZE, " 0 : FLAT STAGE\n 1 : SUMO COMBAT\n 2 : GEOLOGICALLY ACTIVE PVP COMBAT\n 3 : TILTY PVP COMBAT\n 4 : WALLED PVP COMBAT\n 5 : DROPAWAY FLOOR COMBAT\n 6 : PLANET COMBAT\n 7 : MECH COMBAT\n 8 : 1 PLAYER ASTEROID SURVIVAL\n 9 : 2 PLAYER ASTEROID SURVIVAL\n\n");
			drawText(screen, 20, 20+9*TEXTSIZE*11, 0xFFFFFFFF, TEXTSIZE, " M : MANAGE PLAYERS");
			if(sloMo) drawText(screen, 20, 20+9*TEXTSIZE*14, 0xFF0000FF, TEXTSIZE, "TAB: SECRET SLOW STYLE ON!");
			drawText(screen, 20, 20+9*TEXTSIZE*16, 0xFFFFFFFF, TEXTSIZE, netMode?(netMode==2?"CONNECTING":"LISTENING"):"NETWORK INACTIVE");
			drawText(screen, 20, 20+9*TEXTSIZE*17, 0xFFFFFFFF, TEXTSIZE, " H : HOST A GAME");
			if(pIndex[0] != -1) drawText(screen, 20, 20+9*TEXTSIZE*18, 0xFFFFFFFF, TEXTSIZE, " C : CONNECT");
			sprintf(line, " P : PORT : %d", port);
			drawText(screen, 20, 20+9*TEXTSIZE*19, 0xFFFFFFFF, TEXTSIZE, line);
			sprintf(line, " A : ADDR : %s", addressString);
			drawText(screen, 20, 20+9*TEXTSIZE*20, 0xFFFFFFFF, TEXTSIZE, line);
		}else if(inputMode == 1){
			sprintf(line, "PORT : %d", port);
			drawText(screen, 20, 20, 0xFFFFFFFF, TEXTSIZE, line);
		}else if(inputMode == 2){
			sprintf(line, "DESTINATION ADDRESS : %s", addressString);
			drawText(screen, 20, 20, 0xFFFFFFFF, TEXTSIZE, line);
		}else if(inputMode == 3){
			if(netMode){
				drawText(screen, 20, 20, 0xFFFFFFFF, TEXTSIZE, "NO EDITTING ALLOWED WHILE HOSTING");
			}else{
				drawText(screen, 20, 20, 0xFFFFFFFF, TEXTSIZE, "ARROW KEYS TO CHANGE SELECTION");
				drawText(screen, 20, 20+9*TEXTSIZE, 0xFFFFFFFF, TEXTSIZE, "W AND S TO CHANGE COLOR");
				drawText(screen, 20, 20+9*TEXTSIZE*2, 0xFFFFFFFF, TEXTSIZE, "+ AND - TO CHANGE NUMBER");
			}
			int i = 0;
			for(; i < numRequests; i++)
				drawText(screen, 20+TEXTSIZE*9, 20+9*TEXTSIZE*4+9*TEXTSIZE*i, requests[i].color, TEXTSIZE, modeToString(i));
			if(!netMode) drawText(screen, 20, 20+9*TEXTSIZE*4+9*TEXTSIZE*players, 0xFFFFFFFF, TEXTSIZE, ">");
		}
		free(line);
		nothingChanged = 1;
	}else run();
	if(mode && netMode) writeImgs();
	if(frameFlag) *screen = 0xFF8000FF;//Sort of orangy.
	myDrawScreen();
}

static int getDigit(int code){
	if(code == SDL_SCANCODE_0 || code == SDL_SCANCODE_KP_0) return 0;
	if(code >= SDL_SCANCODE_1 && code <= SDL_SCANCODE_9){
		return code-SDL_SCANCODE_1 + 1;
	}
	if(code >= SDL_SCANCODE_KP_1 && code <= SDL_SCANCODE_KP_9){
		return code-SDL_SCANCODE_KP_1 + 1;
	}
	return -1;
}

static void spKeyAction(int bit, Sint8 pressed){
	if(mode == 0){
		if(!pressed) return;
		if(inputMode == 0){
		/*	if(bit == SDL_SCANCODE_RCTRL){
				ais[0] = 1-ais[0];
				nothingChanged = 0;
				return;
			}
			if(bit == SDL_SCANCODE_Q){
				ais[1] = 1-ais[1];
				nothingChanged = 0;
				return;
			}*/
			if(bit == SDL_SCANCODE_TAB){
				sloMo = !sloMo;
				nothingChanged = 0;
				return;
			}
			if(bit == SDL_SCANCODE_ESCAPE){
				if(currentMenu->parent==NULL){
					if(netMode){
						stopHosting();
						nothingChanged = 0;
					}
					return;
				}
				currentMenu = currentMenu->parent;
				nothingChanged = 0;
				return;
			}
			if(bit == SDL_SCANCODE_P){
				inputMode = 1;
				nothingChanged = 0;
				return;
			}
			if(bit == SDL_SCANCODE_A){
				inputMode = 2;
				nothingChanged = 0;
				return;
			}
			if(bit == SDL_SCANCODE_M){
				inputMode = 3;
				players = 0;
				nothingChanged = 0;
				return;
			}
			if(bit == SDL_SCANCODE_C){
				if(pIndex[0] == -1) return;
				myConnect(requests[pIndex[0]].color);
				nothingChanged = 0;
				return;
			}
			if(bit == SDL_SCANCODE_H){
				int playerNumbers[10];
				int numNet = 0;
				int i = 0;
				for(; i < 10; i++){
					if(requests[i].controlMode == -1){
						playerNumbers[numNet++] = i;
					}
				}
				myHost(numNet, playerNumbers);
				nothingChanged = 0;
				return;
			}
			bit = getDigit(bit);
			if(bit > 0 && bit-1 < currentMenu->numItems){
				menuItem* choice = currentMenu->items+bit-1;
				if(choice->menu){
					currentMenu = (menu*)choice->target;
					nothingChanged = 0;
					return;
				}
				players = numRequests;
				(*((level*)choice->target)->func)();
				memset(masterKeys, 0, NUMKEYS*10);
				mode = 1;
				kickNoRoom();
				return;
			}
		} else if (inputMode == 1){
			if(bit == SDL_SCANCODE_BACKSPACE){
				port /= 10;
				nothingChanged = 0;
				return;
			}
			if(bit == SDL_SCANCODE_RETURN || bit == SDL_SCANCODE_KP_ENTER || bit == SDL_SCANCODE_ESCAPE){
				inputMode = 0;
				nothingChanged = 0;
				return;
			}
			bit = getDigit(bit);
			if(bit != -1){
				port = 10*port + bit;
				nothingChanged = 0;
				return;
			}
		} else if (inputMode == 2){
			int len = strlen(addressString);
			if(bit == SDL_SCANCODE_BACKSPACE && len!=0){
				addressString[len-1] = '\0';
				nothingChanged = 0;
				return;
			}
			if(bit == SDL_SCANCODE_RETURN || bit == SDL_SCANCODE_KP_ENTER || bit == SDL_SCANCODE_ESCAPE){
				inputMode = 0;
				nothingChanged = 0;
				return;
			}
			if(len >= 15) return;
			if(bit == SDL_SCANCODE_PERIOD || bit == SDL_SCANCODE_KP_PERIOD){
				addressString[len] = '.';
				addressString[len+1] = '\0';
				nothingChanged = 0;
				return;
			}
			bit = getDigit(bit);
			if(bit != -1){
				addressString[len] = '0'+bit;
				addressString[len+1] = '\0';
				nothingChanged = 0;
				return;
			}
		} else if (inputMode == 3){
			if(bit == SDL_SCANCODE_ESCAPE || bit == SDL_SCANCODE_RETURN || bit == SDL_SCANCODE_KP_ENTER){
				pIndex[0] = -1;
				pIndex[1] = -1;
				int i = 0;
				for(; i < 10; i++){
					if(requests[i].controlMode == 0){
						if(pIndex[0] == -1) pIndex[0] = i;
						else requests[i].controlMode = 2;
					}else if(requests[i].controlMode == 1){
						if(pIndex[1] == -1) pIndex[1] = i;
						else requests[i].controlMode = 2;
					}
				}
				inputMode = 0;
				nothingChanged = 0;
				return;
			}
			if(netMode) return;
			if(bit == SDL_SCANCODE_EQUALS || bit == SDL_SCANCODE_KP_PLUS){
				if(numRequests < 10){
					numRequests++;
					nothingChanged = 0;
				}
				return;
			}
			if(bit == SDL_SCANCODE_MINUS || bit == SDL_SCANCODE_KP_MINUS){
				if(numRequests > 0){
					numRequests--;
					nothingChanged = 0;
					if(players == numRequests) players--;
				}
				return;
			}
			if(bit == SDL_SCANCODE_UP){
				if(players > 0) players--;
				else players = numRequests-1;
				nothingChanged = 0;
				return;
			}
			if(bit == SDL_SCANCODE_DOWN){
				if(players < numRequests-1) players++;
				else players = 0;
				nothingChanged = 0;
				return;
			}
			if(bit == SDL_SCANCODE_LEFT){
				if(requests[players].controlMode > -1){
					if(--requests[players].controlMode == -1) requests[players].color = 0x606060FF;
				}else{
					requests[players].controlMode = 2;
					requests[players].color = getColorFromHue(requests[players].hue);
				}
				nothingChanged = 0;
				return;
			}
			if(bit == SDL_SCANCODE_RIGHT){
				if(requests[players].controlMode < 2){
					if(requests[players].controlMode++ == -1) requests[players].color = getColorFromHue(requests[players].hue);
				}else{
					requests[players].controlMode = -1;
					requests[players].color = 0x606060FF;
				}
				nothingChanged = 0;
				return;
			}
			if(requests[players].controlMode == -1) return;//Aren't allowed to edit hue of network players.
			if(bit == SDL_SCANCODE_W){
				if(requests[players].hue < 372) requests[players].hue+=12;
				else requests[players].hue = 0;
				requests[players].color = getColorFromHue(requests[players].hue);
				nothingChanged = 0;
				return;
			}
			if(bit == SDL_SCANCODE_S){
				if(requests[players].hue > 0) requests[players].hue-=12;
				else requests[players].hue = 372;
				requests[players].color = getColorFromHue(requests[players].hue);
				nothingChanged = 0;
				return;
			}
		}
		return;
	}
	if(bit == SDL_SCANCODE_ESCAPE){
		if(!pressed) return;
		stopField();
		if(netMode) stopHosting();
		mode = 0;
		nothingChanged = 0;
		return;
	}else if(bit==SDL_SCANCODE_MINUS){
		if(pressed && zoom<32768) zoom*=2;
		return;
	}else if(bit==SDL_SCANCODE_EQUALS){
		if(pressed && zoom > 1) zoom/=2;
		return;
	}

	if(pIndex[0] != -1){
		switch(bit){
			case SDL_SCANCODE_LEFT:		masterKeys[pIndex[0]*NUMKEYS + 3] = pressed; return;
			case SDL_SCANCODE_UP:		masterKeys[pIndex[0]*NUMKEYS + 0] = pressed; return;
			case SDL_SCANCODE_RIGHT:	masterKeys[pIndex[0]*NUMKEYS + 1] = pressed; return;
			case SDL_SCANCODE_DOWN:		masterKeys[pIndex[0]*NUMKEYS + 2] = pressed; return;
			case SDL_SCANCODE_RCTRL:	masterKeys[pIndex[0]*NUMKEYS + 4] = pressed; return;
			default: break;
		}
	}
	if(pIndex[1] != -1){
		switch(bit){
			case SDL_SCANCODE_A:		masterKeys[pIndex[1]*NUMKEYS + 3] = pressed; return;
			case SDL_SCANCODE_W:		masterKeys[pIndex[1]*NUMKEYS + 0] = pressed; return;
			case SDL_SCANCODE_D:		masterKeys[pIndex[1]*NUMKEYS + 1] = pressed; return;
			case SDL_SCANCODE_S:		masterKeys[pIndex[1]*NUMKEYS + 2] = pressed; return;
			case SDL_SCANCODE_Q:		masterKeys[pIndex[1]*NUMKEYS + 4] = pressed; return;
			default: return;
		}
	}
}

int main(int argc, char** argv){
	menu topMenu;
	currentMenu = &topMenu;
	topMenu.parent = NULL;
	topMenu.numItems = 9;
	topMenu.items = malloc(9*sizeof(menuItem));
	//menu* asteroidsMenu = addMenuMenu(&topMenu, 0, 2, "ASTEROID SURVIVAL...");
	menu* planetsMenu = addMenuMenu(&topMenu, 0, 2, "PLANET STAGES...");
	addMenuLevel(&topMenu, 1, &lvltest, "FLAT STAGE");
	addMenuLevel(&topMenu, 2, &lvlsumo, "SUMO");
	addMenuLevel(&topMenu, 3, &lvltipsy, "UNSTABLE STAGE");
	addMenuLevel(&topMenu, 4, &lvltilt, "TILTY STAGE");
	addMenuLevel(&topMenu, 5, &lvlswing, "WALLED STAGE");
	addMenuLevel(&topMenu, 6, &lvldrop, "DROPAWAY FLOOR");
	addMenuLevel(&topMenu, 7, &lvlmech, "MECHS");
	addMenuLevel(&topMenu, 8, &lvlsurvive, "ASTEROID SURVIVAL");

	addMenuLevel(planetsMenu, 0, &lvlplanet, "SINGLE PLANET");
	addMenuLevel(planetsMenu, 1, &lvl3rosette, "3-ROSETTE");

	initNetworking();

	numRequests = 2;
	srand(time(NULL));
	int i = 0;
	for(; i < 10; i++){
		requests[i].hue=(long int)rand() * 32 / RAND_MAX * 12;
		requests[i].color=getColorFromHue(requests[i].hue);
		requests[i].controlMode = 2;
	}
	requests[0].controlMode = 1;
	requests[1].controlMode = 0;
	#ifndef WINDOWS
	struct timespec t;
	t.tv_sec = 0;
	time_t lastTime = 0;
	#endif
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	window = SDL_CreateWindow("Bounce Brawl", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 750, 750, 0);
	if(window == NULL){
		fputs("No SDL2 window.\n", stderr);
		fputs(SDL_GetError(), stderr);
		SDL_Quit();
		return 1;
	}
	render = SDL_CreateRenderer(window, -1, 0);
	if(render == NULL){
		fputs("No SDL2 renderer.\n", stderr);
		fputs(SDL_GetError(), stderr);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}
	texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 750, 750);
	if(texture == NULL){
		fputs("No SDL2 texture.\n", stderr);
		fputs(SDL_GetError(), stderr);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}
	screen = malloc(750*750*4);
	//screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 500, 500, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
	int running=1;
	while(running){
		if(netMode) readKeys();
		paint();//Also runs the thing if relevant.

		long int sleep = (sloMo?.15:0.015 - difftime(time(NULL), lastTime))*1000000000;
		if(sleep > 0){
			frameFlag = 0;
			t.tv_nsec = sleep;
			nanosleep(&t, &t);
		} else frameFlag = 1;
		time(&lastTime);

		SDL_Event evnt;
		while(SDL_PollEvent(&evnt)){
			if(evnt.type == SDL_QUIT)		running = 0;
			else if(evnt.type == SDL_KEYDOWN)	spKeyAction(evnt.key.keysym.scancode, 1);
			else if (evnt.type == SDL_KEYUP)	spKeyAction(evnt.key.keysym.scancode, 0);
			else if (evnt.type == SDL_WINDOWEVENT)	nothingChanged = 0;//Just to be safe, in case something was occluded.
		}
	}
	if(netMode) stopHosting();
	stopNetworking();
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
