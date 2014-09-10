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
#include "task.h"
#include "gui.h"

#ifdef WINDOWS
#include <windows.h>
#endif

typedef struct{
	char menu;
	void* target;
	char* achievementText;
	char achievementUnlocked;
	char* text;
} menuItem;

typedef struct menu{
	struct menu* parent;
	int numItems;
	menuItem* items;
}menu;

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
int pKeys[2][NUMKEYS] = {{SDLK_w, SDLK_d, SDLK_s, SDLK_a, SDLK_x, SDLK_z},\
	{SDLK_UP, SDLK_RIGHT, SDLK_DOWN, SDLK_LEFT, SDLK_RCTRL, SDLK_RSHIFT}};
int otherKeys[2] = {SDLK_EQUALS, SDLK_MINUS};

char mode = 0, cheats = 0;
static int running = 1;
static char inputMode = 0;
static char achievementView = 0;
static char nothingChanged = 0;
char frameCount = SHOWEVERYNTHFRAME;
//static char frameFlag;

FILE* logFile;

static void freeMenu(menu* who){
	int i = who->numItems-1;
	for(; i >= 0; i--) free(who->items[i].target);
	free(who->items);
}

static menu* addMenuMenu(menu* parent, int numItems, char* text){
	menu* ret = malloc(sizeof(menu));
	ret->parent = parent;
	ret->numItems = 0;
	ret->items = malloc(numItems*sizeof(menuItem));
	menuItem* item = parent->items+parent->numItems++;
	item->menu = 1;
	item->target = ret;
	item->text = text;
	item->achievementText = "COMPLETE ALL SUB-ACHIEVEMENTS";
	item->achievementUnlocked = 0;
	return ret;
}

static void addMenuLevel(menu* where, void (*func)(), char* text, char* achievementText){
	menuItem* item = where->items+where->numItems++;
	item->menu = 0;
	item->target = malloc(sizeof(void (*)()));
	*((void (**)())item->target) = func;
	item->text = text;
	item->achievementText = achievementText;
	item->achievementUnlocked = 0;
}

void myDrawScreenNoClear(){
	SDL_GL_SwapWindow(window);
}

void myDrawScreen(){
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
			return " WIMPY COMBAT AI";
		case 3:
			return " COMBAT AI";
		case 4:
			return " DIEHARD COMBAT AI";
		case 5:
			return " EXPERIMENTAL SPACE AI";
		case 6:
			return " DIEHARD EXPERIMENTAL SPACE AI";
		default:
			return " Error! Danger! Augh!";
	}
}

static inline void simpleDrawText(int line, char* text){
	drawText(20-width2, 20+TEXTSIZE*9*line+height2, TEXTSIZE, text);
}

static void paint(){
	static char line[40];
	GLenum error = glGetError();
	if(error){
		fputs((const char*)gluErrorString(error), logFile);
		fputc('\n', logFile);
		fflush(logFile);
	}
	if(mode == 0){
		if(nothingChanged) return;
//		boxColor(render, 0, 0, 500, 500, 0x000000FF);
//		memset(screen, 0, 750*750*4);
//		SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
//		SDL_Rect a = {.x=0, .y=0, .w=500, .h=500};
//		SDL_RenderFillRect(render, &a);
		setColorWhite();
		simpleDrawText(30, "ESC: CANCEL / GO BACK");
		if(inputMode == -1){
			simpleDrawText(3, "REALLY QUIT? PRESS ANY KEY TO EXIT");
		}else if(inputMode == 0){
			int i = 0;
			strcpy(line, "   : ");
			if(achievementView){
				for(; i < currentMenu->numItems; i++){
					if(currentMenu->items[i].achievementUnlocked){
						setColorFromHue((i%6)*64);
						line[1] = 15;
					}else{
						setColorWhite();
						line[1] = ' ';
					}
					strcpy(line + 5, currentMenu->items[i].achievementText);
					simpleDrawText(i, line);
				}
				simpleDrawText(11, " V : REGULAR VIEW");
			}else{
				for(; i < currentMenu->numItems; i++){
					strcpy(line + 5, currentMenu->items[i].text);
					line[1] = '1' + i;
					simpleDrawText(i, line);
				}
				simpleDrawText(11, " V : ACHEIVEMENT VIEW");
			}
			simpleDrawText(12, " M : MANAGE PLAYERS");
			simpleDrawText(13, " K : SET KEYS");
			simpleDrawText(16, netMode?"LISTENING":"NETWORK INACTIVE");
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
			if(cheats&CHEAT_LOCK) simpleDrawText(25, "TAB: CONTROLS LOCKED");
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
		nothingChanged = 1;
		myDrawScreen();
	}else{
		run();
		if(--frameCount == 0){
			draw();
			runTask(&firstTask);
			if(netMode) writeImgs();
			myDrawScreen();
			frameCount = SHOWEVERYNTHFRAME;
		}else{
			runTask(&firstTask);
		}
	}
//	if(frameFlag) *screen = 0xFF8000FF;//Sort of orangy.
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
		if(inputMode == -1){
			if(bit == SDLK_ESCAPE){
				inputMode = 0;
				nothingChanged = 0;
				return;
			}
			running = 0;
			return;
		}else if(inputMode == 0){
			if(bit == SDLK_TAB){
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
				nothingChanged = 0;
				if(currentMenu->parent==NULL){
					if(netMode) stopHosting();
					else inputMode = -1;
					return;
				}
				currentMenu = currentMenu->parent;
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
				myConnect();
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_v){
				achievementView = !achievementView;
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_h){
				char *playerNumbers = calloc(1, 10);
				int numNet = 0;
				int i = 0;
				for(; i < 10; i++){
					if(requests[i].controlMode == -1){
						numNet++;
						playerNumbers[i] = 1; // :'(
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
				(**((void (**)())choice->target))();
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
					requests[players].controlMode = 6;
					requests[players].color = getColorFromHue(requests[players].hue);
				}
				nothingChanged = 0;
				return;
			}
			if(bit == SDLK_RIGHT){
				if(requests[players].controlMode < 6){
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
	}else if(bit == SDLK_TAB){
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
	logFile = fopen("log.txt", "w");
	fputs("Everything looks good from here\n", logFile);
	menu topMenu;
	currentMenu = &topMenu;
	topMenu.parent = NULL;
	topMenu.numItems = 0;
	topMenu.items = malloc(7*sizeof(menuItem));
	menu* planetsMenu = addMenuMenu(&topMenu, 3, "PLANET STAGES...");
	menu* flatMenu    = addMenuMenu(&topMenu, 4, "FLAT STAGES...");
	menu* suspendedMenu    = addMenuMenu(&topMenu, 3, "SUSPENDED STAGES...");
	menu* mechMenu    = addMenuMenu(&topMenu, 3, "MECHS...");
	addMenuLevel(&topMenu, &lvlsumo, "SUMO", "DESTRUCTION");
//	addMenuLevel(&topMenu, &lvltipsy, "UNSTABLE STAGE");
//	addMenuLevel(&topMenu, &lvltilt, "TILTY STAGE");
	addMenuLevel(&topMenu, &lvlcave, "CAVE", "SPELUNKER");
	addMenuLevel(&topMenu, &lvltutorial, "TUTORIAL", "NO SHIRT, NO SHOES...");

	addMenuLevel(planetsMenu, &lvlplanet, "SINGLE PLANET", "SPAAAAAAAAACE!!!");
	addMenuLevel(planetsMenu, &lvl3rosette, "3-ROSETTE", "POTENTIAL WELL");
	addMenuLevel(planetsMenu, &lvlbigplanet, "BIG PLANET", "EVERYTHING BUT THE SEED");

	addMenuLevel(flatMenu, &lvltest, "PLAIN STAGE", "PACIFISM");
	addMenuLevel(flatMenu, &lvlsurvive, "ASTEROID SURVIVAL", "BETTER THAN THE DINOSAURS");
	addMenuLevel(flatMenu, &lvlbuilding, "BUILDING STAGE", "MOUNTAINEER");
	addMenuLevel(flatMenu, &lvlboulder, "BOULDER", "GRAVEL");

	addMenuLevel(mechMenu, &lvlmech, "MAN VS MECH", "BEACHED");
	addMenuLevel(mechMenu, &lvlmechgun, "GUN VS MECH", "MOAR PACIFISM");
	addMenuLevel(mechMenu, &lvlmechmech, "MECH VS MECH", "CHANGE PLACES!");

	addMenuLevel(suspendedMenu, &lvlgardens, "HANGING GARDENS", "FLOOD");
	addMenuLevel(suspendedMenu, &lvlswing, "WALLED STAGE", "MOAR DESTRUCTION");
	addMenuLevel(suspendedMenu, &lvldrop, "DROPAWAY FLOOR", "I CAN HAZ DESTRUCTION?");
	
	fputs("Menu Created\n", logFile);

	initNetworking();
	fputs("Networking Initialized\n", logFile);

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
	fputs("Player specs and timing initialized\n", logFile);
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	//SDL_ClearError();
//	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);///Maybe someday these will be useful for asking for a newer version of opengl
//	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	fputs(SDL_GetError(), logFile);
	fputc('\n', logFile);
	SDL_ClearError();
	window = SDL_CreateWindow("Bounce Brawl", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 750, 750, SDL_WINDOW_OPENGL);
	fputs(SDL_GetError(), logFile);
	fputc('\n', logFile);
	SDL_ClearError();
	if(window == NULL){
		fputs("No SDL2 window.\n", stderr);
		fputs(SDL_GetError(), stderr);
		SDL_Quit();
		return 1;
	}
	fputs("Created Window\n", logFile);
	width2 = 350;
	height2 = -350;
	SDL_GLContext context = SDL_GL_CreateContext(window);
	fputs(SDL_GetError(), logFile);
	fputc('\n', logFile);
	SDL_ClearError();
	fputs("Created GL Context\n", logFile);
	initGfx(logFile);
	glClearColor(0, 0, 0, 1);
	fputs(SDL_GetError(), logFile);
	fputc('\n', logFile);
	SDL_ClearError();
	fputs("Intialized Graphics, Entering Main Loop\n", logFile);
	fflush(logFile);
	while(running){
		if(netMode) readKeys();
		paint();///Also runs the thing if relevant.
		if(!(cheats & CHEAT_SPEED && mode) && frameCount==SHOWEVERYNTHFRAME){
#ifndef WINDOWS
			clock_gettime(CLOCK_MONOTONIC, &otherTime);
			long int sleep = (long int)((cheats&CHEAT_SLOMO)?250000000*SPEEDFACTOR*SHOWEVERYNTHFRAME:25000000*SPEEDFACTOR*SHOWEVERYNTHFRAME) - (otherTime.tv_nsec-lastTime.tv_nsec+1000000000l*(otherTime.tv_sec-lastTime.tv_sec));
			if(sleep > 0){
//				frameFlag = 0;
				t.tv_nsec = sleep;
				nanosleep(&t, NULL);
			}// else frameFlag = 1;
			clock_gettime(CLOCK_MONOTONIC, &lastTime);
#else
			largeInt.LowPart = lastTime.dwLowDateTime;
			largeInt.HighPart = lastTime.dwHighDateTime;
			largeInt.QuadPart += (cheats&CHEAT_SLOMO)?2500000*SPEEDFACTOR*SHOWEVERYNTHFRAME:250000*SPEEDFACTOR*SHOWEVERYNTHFRAME;
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
	freeMenu(flatMenu); // Do these from the bottom of the menu tree up
	freeMenu(mechMenu);
	freeMenu(suspendedMenu);
	freeMenu(planetsMenu);
	freeMenu(&topMenu);
	fclose(logFile);
	if(netMode) stopHosting();
	stopNetworking();
	quitGfx();
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
#ifdef WINDOWS
	CloseHandle(hTimer);
#endif
	SDL_Quit();
	return 0;
}
