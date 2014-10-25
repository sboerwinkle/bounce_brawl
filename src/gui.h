#define NUMKEYS 6
#define TEXTSIZE 2

extern void myDrawScreenNoClear();

extern void myDrawScreen();

extern int players;
//Stores which player each of the two local guys is
extern int pIndex[2];
//Stores the keybindings
extern int pKeys[2][NUMKEYS];
extern int otherKeys[2];

//Stores the keys which each player. AIs or the keyboard writes here, TGC reads from here.
extern char masterKeys[NUMKEYS * 10];

typedef struct {
	uint16_t hue;
	uint32_t color;
	//Indicates if it's a human, a networked player, which kind of AI, etc.
	int controlMode;
} playerRequest;

extern playerRequest requests[10];

extern char mode;

extern char cheats;

#define CHEAT_NUCLEAR 1
#define CHEAT_SLOMO   2
#define CHEAT_LOCK    4
#define CHEAT_SPEED   8
#define CHEAT_COLORS 16

extern char nothingChanged;

extern char frameCount;

extern FILE *logFile;
