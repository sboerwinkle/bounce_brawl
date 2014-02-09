
extern int width2, height2;

extern void initGfx();

extern void setColorFromHue(uint16_t hue);

extern uint32_t getColorFromHue(uint16_t hue);

extern void setColorFromHex(uint32_t hex);

extern void setColorWhite();

extern void drawRectangle(float x1, float y1, float x2, float y2);

extern void drawBox(float x1, float y1, float x2, float y2);

extern void drawLine(float x1, float y1, float x2, float y2);

extern void drawCircle(float cx, float cy, float r);
