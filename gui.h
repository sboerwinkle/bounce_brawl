#define NUMKEYS 6
#define TEXTSIZE 2
//If these are changed, change gui.c

//extern uint32_t* screen;
extern void myDrawScreen();

extern int players;
extern int pIndex[2];

extern char masterKeys[NUMKEYS*10];

typedef struct{
	uint16_t hue;
	uint32_t color;
	int controlMode;
}playerRequest;

extern playerRequest requests[10];

extern char mode;

extern char cheats;

#define CHEAT_NUCLEAR 1
#define CHEAT_SLOMO   2
#define CHEAT_LOCK    4
#define CHEAT_SPEED   8
#define CHEAT_COLORS 16
