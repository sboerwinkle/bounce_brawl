extern char netMode;
extern int port;
extern char* addressString;

extern void initNetworking();
extern void stopNetworking();

extern void myConnect();
extern void myHost(int max, char* playerNumbers);
extern void kickNoRoom();
extern void readKeys();
extern void stopHosting();

extern void addNetCircle(short x, short y, int r);
extern void addNetLine(uint16_t dest, uint8_t hue);
extern int  addNetLineCircle(uint16_t ix);
extern void setNetLineCircleNumber(int pos, uint8_t num);
extern void addNetPlayerCircle(uint16_t ix, short x, short y, uint32_t color);
extern void addNetTool(int ix, int color);
extern void removeNetLineCircle();
extern void writeImgs();
