#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "gfx.h"
#include "structs.h"
#include "font.h"
#include "levels.h"
#include "field.h"
#include "networking.h"
#include "gui.h"

#ifdef WINDOWS
#include <windows.h>
#endif

typedef struct{
	int highscore;
	void (*func)();
} level;

typedef struct{
	char menu;
	void* target;
	char* text;
} menuItem;

struct menu{
	struct menu* parent;
	int numItems;
	menuItem* items;
};
typedef struct menu menu;

menu* currentMenu;

static SDL_Window* window;
//static SDL_Renderer* render;
//static SDL_Texture* texture;
//uint32_t *screen;

int players = 0;
static int numRequests;
playerRequest requests[10];

char masterKeys[NUMKEYS*10];
int pIndex[] = {0, 1};
int pKeys[2][6] = {{SDLK_w, SDLK_d, SDLK_s, SDLK_a, SDLK_q, SDLK_1},\
	{SDLK_UP, SDLK_RIGHT, SDLK_DOWN, SDLK_LEFT, SDLK_RCTRL, SDLK_RSHIFT}};
int otherKeys[2] = {SDLK_EQUALS, SDLK_MINUS};

char mode = 0, cheats = 0;
static char inputMode = 0;
static char nothingChanged = 0;
//static char frameFlag = 0;

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

void myDrawScreen(){
//	if(SDL_UpdateTexture(texture, NULL, screen, 750*4) < 0) puts(SDL_GetError());
//	if(SDL_RenderClear(render) < 0) puts(SDL_GetError());
//	if(SDL_RenderCopy(render, texture, NULL, NULL) < 0) puts(SDL_GetError());
//	SDL_RenderPresent(render);
	SDL_GL_SwapWindow(window);
	glClear(GL_COLOR_BUFFER_BIT);
}

static char* modeToString(int ix){
	switch(requests[ix].controlMode){
		case -1:
			return " NETWORKED PLAYER";
		case 0:
			return " PLAYER 1";
		case 1:
			return " PLAYER 2";
		case 2:
			return " COMBAT AI";
		case 3:
			return " DIEHARD COMBAT AI";
		case 4:
			return " EXPERIMENTAL SPACE AI";
		case 5:
			return " DIEHARD EXPERIMENTAL SPACE AI";
		default:
			return " Error! Danger! Augh!";
	}
}

static inline void simpleDrawText(int line, char* text){
	drawText(20-width2, 20+TEXTSIZE*9*line+height2, TEXTSIZE, text);
}

static void paint(){
	GLenum error = glGetError();
	if(error) fputs((const char*)gluErrorString(error), stderr);
	if(mode == 0){
		if(nothingChanged) return;
//		boxColor(render, 0, 0, 500, 500, 0x000000FF);
//		memset(screen, 0, 750*750*4);
//		SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
//		SDL_Rect a = {.x=0, .y=0, .w=500, .h=500};
//		SDL_RenderFillRect(render, &a);
		char* line = malloc(40*sizeof(char));
		setColorWhite();
		simpleDrawText(30, "ESC: CANCEL / GO BACK");
		if(inputMode == 0){
			int i = 0;
			for(; i < currentMenu->numItems; i++){
				sprintf(line, " %d : %s", i+1, currentMenu->items[i].text);
				simpleDrawText(i, line);
			}
			simpleDrawText(11, " M : MANAGE PLAYERS");
			simpleDrawText(12, " K : SET KEYS");
			simpleDrawText(16, netMode?(netMode==2?"CONNECTING":"LISTENING"):"NETWORK INACTIVE");
			simpleDrawText(17, " H : HOST A GAME");
			if(pIndex[0] != -1) simpleDrawText(18, " C : CONNECT");
			sprintf(line, " P : PORT : %d", port);
			simpleDrawText(19, line);
			sprintf(line, " A : ADDR : %s", addressString);
			simpleDrawText(20, line);

			setColorFromHue(256);
			if(cheats&CHEAT_SLOMO) simpleDrawText(23, "SUP: SECRET SLOW STYLE!");
			if(cheats&CHEAT_NUCLEAR){
				simpleDrawText(24, "F6 : I SAID I'M");
				drawText(20-width2+TEXTSIZE*9*16, 20+TEXTSIZE*(9*24-4*0.3)+height2, TEXTSIZE*1.3, "NUCLEAR!");
			}
			if(cheats&CHEAT_LOCK) simpleDrawText(25, "CAP: CONTROLS LOCKED");
			if(cheats&CHEAT_SPEED) simpleDrawText(26, "F11: SECRET SUPER SPEED STYLE!");
			if(cheats&CHEAT_COLORS) simpleDrawText(27, " \\ : COLORLESS MODE");
			setColorWhite();
		}else if(inputMode == 1){
			sprintf(line, "PORT : %d", port);
			simpleDrawText(0, line);
		}else if(inputMode == 2){
			sprintf(line, "DESTINATION ADDRESS : %s", addressString);
			simpleDrawText(0, line);
		}else if(inputMode == 3){
			if(netMode){
				simpleDrawText(0, "NO EDITTING ALLOWED WHILE HOSTING");
			}else{
				simpleDrawText(0, "ARROW KEYS TO CHANGE SELECTION");
				simpleDrawText(1, "           OR PLAYER TYPE");
				simpleDrawText(2, "W AND S TO CHANGE COLOR");
				simpleDrawText(3, "+ AND - TO CHANGE NUMBER");
			}
			int i = 0;
			for(; i < numRequests; i++){
				setColorFromHex(requests[i].color);
				simpleDrawText(5+i, modeToString(i));
			}
			if(!netMode){
				setColorWhite();
				simpleDrawText(5+players, ">");
			}
		}else if(inputMode == 4){
			simpleDrawText(0, "ARROW KEYS TO CHANGE SELECTION");
			simpleDrawText(1, "SPACE OR ENTER TO SET KEY");
			char *(text[NUMKEYS]) = {"UP:      ", "RIGHT:   ", "DOWN:    ", "LEFT:    ", "INTERACT:", "ACTION:  "};
			int i, j;
			for(j = 0; j < 2; j++)
				for(i = 0; i < NUMKEYS; i++){
					sprintf(line, " P%d %s %s", j+1, text[i], SDL_GetKeyName(pKeys[j][i]));
					setColorFromHue(20*i);
					simpleDrawText(3+i+j*NUMKEYS, line);
				}
			setColorFromHue(0);
			sprintf(line, " ZOOM IN:     %s", SDL_GetKeyName(otherKeys[0]));
			simpleDrawText(3+2*NUMKEYS, line);
			setColorFromHue(20);
			sprintf(line, " ZOOM OUT:    %s", SDL_GetKeyName(otherKeys[1]));
			simpleDrawText(3+2*NUMKEYS+1, line);
			setColorWhite();
			simpleDrawText(3+(players&(~1024)), (players&1024)?"=":">");
		}
		free(line);
		nothingChanged = 1;
	}else run();
	if(mode && netMode) writeImgs();
//	if(frameFlag) *screen = 0xFF8000FF;//Sort of orangy.
	myDrawScreen();
}

static int getDigit(int code){
	if(code == SDLK_0 || code == SDLK_KP_0) return 0;
	if(code >= SDLK_1 && code <= SDLK_9){
		return code-SDLK_1 + 1;
	}
	if(code >= SDLK_KP_1 && code <= SDLK_KP_9){
		return code-SDLK_KP_1 + 1;
	}
	return -1;
}

static void spKeyAction(int bit, char pressed){
	if(mode == 0){
		if(!pressed) return;
		if(inputMode == 0){
			if(bit == SDLK_CAPSLOCK){
				cheats ^= CHEAT_LOCK;
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_LGUI || bit == SDLK_RGUI){
				cheats ^= CHEAT_SLOMO;
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_BACKSLASH){
				cheats ^= CHEAT_COLORS;
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_F6){
				cheats ^= CHEAT_NUCLEAR;
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_F11){
				cheats ^= CHEAT_SPEED;
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_ESCAPE){
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
			if(bit == SDLK_p){
				inputMode = 1;
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_a){
				inputMode = 2;
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_m){
				inputMode = 3;
				players = 0;
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_k){
				inputMode = 4;
				players = 0;
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_c){
				if(pIndex[0] == -1) return;
				myConnect(requests[pIndex[0]].color);
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_h){
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
			if(bit == SDLK_BACKSPACE){
				port /= 10;
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_RETURN || bit == SDLK_KP_ENTER || bit == SDLK_ESCAPE){
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
			if(bit == SDLK_BACKSPACE && len!=0){
				addressString[len-1] = '\0';
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_RETURN || bit == SDLK_KP_ENTER || bit == SDLK_ESCAPE){
				inputMode = 0;
				nothingChanged = 0;
				return;
			}
			if(len >= 15) return;
			if(bit == SDLK_PERIOD || bit == SDLK_KP_PERIOD){
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
			if(bit == SDLK_ESCAPE || bit == SDLK_RETURN || bit == SDLK_KP_ENTER){
				pIndex[0] = -1;
				pIndex[1] = -1;
				int i = 0;
				for(; i < numRequests; i++){
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
			if(bit == SDLK_EQUALS || bit == SDLK_KP_PLUS){
				if(numRequests < 10){
					numRequests++;
					nothingChanged = 0;
				}
				return;
			}
			if(numRequests == 0) return;
			if(bit == SDLK_MINUS || bit == SDLK_KP_MINUS){
				numRequests--;
				if(players == numRequests && numRequests != 0) players--;
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_UP){
				if(players > 0) players--;
				else players = numRequests-1;
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_DOWN){
				if(players < numRequests-1) players++;
				else players = 0;
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_LEFT){
				if(requests[players].controlMode > -1){
					if(--requests[players].controlMode == -1) requests[players].color = 0x606060FF;
				}else{
					requests[players].controlMode = 5;
					requests[players].color = getColorFromHue(requests[players].hue);
				}
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_RIGHT){
				if(requests[players].controlMode < 5){
					if(requests[players].controlMode++ == -1) requests[players].color = getColorFromHue(requests[players].hue);
				}else{
					requests[players].controlMode = -1;
					requests[players].color = 0x606060FF;
				}
				nothingChanged = 0;
				return;
			}
			if(requests[players].controlMode == -1) return;//Aren't allowed to edit hue of network players.
			if(bit == SDLK_w){
				if(requests[players].hue < 372) requests[players].hue+=12;
				else requests[players].hue = 0;
				requests[players].color = getColorFromHue(requests[players].hue);
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_s){
				if(requests[players].hue > 0) requests[players].hue-=12;
				else requests[players].hue = 372;
				requests[players].color = getColorFromHue(requests[players].hue);
				nothingChanged = 0;
				return;
			}
		} else if (inputMode == 4){
			if(players&1024){
				players &= ~1024;
				if(bit != SDLK_ESCAPE){
					if(players<2*NUMKEYS)
						pKeys[players/NUMKEYS][players%NUMKEYS] = bit;
					else
						otherKeys[players-2*NUMKEYS] = bit;
				}
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_KP_ENTER || bit == SDLK_RETURN || bit == SDLK_SPACE){
				players |= 1024;
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_ESCAPE){
				inputMode = 0;
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_UP){
				if(players > 0) players--;
				else players = NUMKEYS*2+2-1;
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_DOWN){
				if(players < NUMKEYS*2+2-1) players++;
				else players = 0;
				nothingChanged = 0;
				return;
			}
		}
		return;
	}
	if(bit == SDLK_ESCAPE){
		if(!pressed) return;
		stopField();
		if(netMode) stopHosting();
		mode = 0;
		nothingChanged = 0;
		return;
	}else if(bit == SDLK_LGUI || bit == SDLK_RGUI){
		if(pressed)
			cheats ^= CHEAT_SLOMO;
		return;
	}else if(bit == SDLK_F11){
		if(pressed)
			cheats ^= CHEAT_SPEED;
		return;
	}else if(bit == SDLK_CAPSLOCK){
		if(pressed) cheats ^= CHEAT_LOCK;
		return;
	}else if(cheats & CHEAT_LOCK) return;
	else if(bit==otherKeys[1]){
		if(pressed && zoom<32768) zoom*=2;
		return;
	}else if(bit==otherKeys[0]){
		if(pressed && zoom > 1) zoom/=2;
		return;
	}
	int i, j;
	for(j=0; j < 2; j++){
		if(pIndex[j] == -1) continue;
		for(i=0; i<NUMKEYS; i++)
			if(pKeys[j][i]==bit){
				masterKeys[pIndex[j]*NUMKEYS + i] = pressed;
				return;
			}
	}
}

int main(int argc, char** argv){
	menu topMenu;
	currentMenu = &topMenu;
	topMenu.parent = NULL;
	topMenu.numItems = 8;
	topMenu.items = malloc(topMenu.numItems*sizeof(menuItem));
	//menu* asteroidsMenu = addMenuMenu(&topMenu, 0, 2, "ASTEROID SURVIVAL...");
	menu* planetsMenu = addMenuMenu(&topMenu, 0, 2, "PLANET STAGES...");
	menu* flatMenu    = addMenuMenu(&topMenu, 1, 4, "FLAT STAGES...");
	addMenuLevel(&topMenu, 2, &lvlsumo, "SUMO");
//	addMenuLevel(&topMenu, 3, &lvltipsy, "UNSTABLE STAGE");
//	addMenuLevel(&topMenu, 4, &lvltilt, "TILTY STAGE");
	addMenuLevel(&topMenu, 3, &lvlswing, "WALLED STAGE");
	addMenuLevel(&topMenu, 4, &lvldrop, "DROPAWAY FLOOR");
	menu* mechMenu    = addMenuMenu(&topMenu, 5, 3, "MECHS...");
	addMenuLevel(&topMenu, 6, &lvlcave, "CAVE");
	addMenuLevel(&topMenu, 7, &lvltutorial, "TUTORIAL");

	addMenuLevel(planetsMenu, 0, &lvlplanet, "SINGLE PLANET");
	addMenuLevel(planetsMenu, 1, &lvl3rosette, "3-ROSETTE");

	addMenuLevel(flatMenu, 0, &lvltest, "PLAIN STAGE");
	addMenuLevel(flatMenu, 1, &lvlsurvive, "ASTEROID SURVIVAL");
	addMenuLevel(flatMenu, 2, &lvlbuilding, "BUILDING STAGE");
	addMenuLevel(flatMenu, 3, &lvlexperiment, "EXPERIMENTAL STAGE");

	addMenuLevel(mechMenu, 0, &lvlmech, "MAN VS MECH");
	addMenuLevel(mechMenu, 1, &lvlmechmech, "MECH VS MECH");
	addMenuLevel(mechMenu, 2, &lvlmechgun, "GUN VS MECH");

	initNetworking();

	numRequests = 2;
	srand(time(NULL));
	int i = 0;
	for(; i < 10; i++){
		requests[i].hue=(long int)rand() * 32 / RAND_MAX * 12;
		requests[i].color=getColorFromHue(requests[i].hue);
		requests[i].controlMode = 2;
	}
	requests[0].controlMode = 0;
	requests[1].controlMode = 1;
#ifndef WINDOWS
	struct timespec t;
	t.tv_sec = 0;
	struct timespec lastTime = {.tv_sec = 0, .tv_nsec = 0};
	struct timespec otherTime = {.tv_sec = 0, .tv_nsec = 0};
#else
	HANDLE hTimer = CreateWaitableTimer(NULL, 1, NULL);
	FILETIME lastTime = {.dwLowDateTime = 0, .dwHighDateTime = 0};
	LARGE_INTEGER largeInt;
#endif
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	SDL_ClearError();
//	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);///Maybe someday these will be useful for asking for a newer version of opengl
//	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	puts(SDL_GetError());
	SDL_ClearError();
	window = SDL_CreateWindow("Bounce Brawl", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 750, 750, SDL_WINDOW_OPENGL);
	puts(SDL_GetError());
	SDL_ClearError();
	if(window == NULL){
		fputs("No SDL2 window.\n", stderr);
		fputs(SDL_GetError(), stderr);
		SDL_Quit();
		return 1;
	}
	width2 = 350;
	height2 = -350;
/*	render = SDL_CreateRenderer(window, -1, 0);
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
	SDL_SetRenderDrawColor(render, 0, 0, 0, 255);*/
	SDL_GLContext context = SDL_GL_CreateContext(window);
	puts(SDL_GetError());
	SDL_ClearError();
	initGfx();
	glClearColor(0, 0, 0, 1);
	int running=1;
	while(running){
		if(netMode) readKeys();
		paint();///Also runs the thing if relevant.
		if(!(cheats & CHEAT_SPEED)){
#ifndef WINDOWS
			clock_gettime(CLOCK_MONOTONIC, &otherTime);
			long int sleep = ((cheats&CHEAT_SLOMO)?250000000:25000000) - (otherTime.tv_nsec-lastTime.tv_nsec+1000000000l*(otherTime.tv_sec-lastTime.tv_sec));
			if(sleep > 0){
//				frameFlag = 0;
				t.tv_nsec = sleep;
				nanosleep(&t, &t);
			}// else frameFlag = 1;
			clock_gettime(CLOCK_MONOTONIC, &lastTime);
#else
			largeInt.LowPart = lastTime.dwLowDateTime;
			largeInt.HighPart = lastTime.dwHighDateTime;
			largeInt.QuadPart += (cheats&CHEAT_SLOMO)?2500000:250000;
			SetWaitableTimer(hTimer, &largeInt, 0, NULL, NULL, 0);
			WaitForSingleObject(hTimer, INFINITE);
			GetSystemTimeAsFileTime(&lastTime);
#endif
		}

		SDL_Event evnt;
		while(SDL_PollEvent(&evnt)){
			if(evnt.type == SDL_QUIT)		running = 0;
			else if(evnt.type == SDL_KEYDOWN)	spKeyAction(evnt.key.keysym.sym, 1);
			else if (evnt.type == SDL_KEYUP)	spKeyAction(evnt.key.keysym.sym, 0);
			else if (evnt.type == SDL_WINDOWEVENT)	nothingChanged = 0;//Just to be safe, in case something was occluded.
		}
	}
	if(netMode) stopHosting();
	stopNetworking();
	SDL_DestroyWindow(window);
#ifdef WINDOWS
	CloseHandle(hTimer);
#endif
	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}
