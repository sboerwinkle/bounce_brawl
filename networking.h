extern Sint8 netMode;
extern int port;
extern char* addressString;

extern void initNetworking();
extern void stopNetworking();

extern void myConnect(Uint32 color);
extern void myHost(int max, int* playerNumbers);
extern void kickNoRoom();
extern void readKeys();
extern void stopHosting();

extern void addNetCircle(short x, short y, int r);
extern void addNetLine(Uint16 dest, Uint8 hue);
extern int  addNetLineCircle(Uint16 ix);
extern void setNetLineCircleNumber(int pos, Uint8 num);
extern void addNetPlayerCircle(Uint16 ix, Uint32 color);
extern void addNetTool(int ix, int color);
extern void removeNetLineCircle();
extern void writeImgs();
