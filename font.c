#include "fontData.h"
#include <SDL2/SDL.h>
#include "gfx.h"

void drawText(int X, int Y, float size, char* string){
	float x = (float)X/width2;
	float y = (float)Y/height2;
	size /= width2;
	int j = -1;
	int i = -1;
	int y2;
	int x2;
	register uint8_t row;
	register uint8_t test;
	float tempx, tempy;
	while(string[++j] != '\0'){
		i++;
		if(string[j] == '\n'){
			i = -1;
			y-=size*9;
			continue;
		}
		tempy = y;
		for(y2 = 0; y2<8; y2++){
			row = gfxPrimitivesFontdata[8*string[j]+y2];
			test = 128;
			tempx = x+i*9*size;
			for(x2 = 0; x2<8; x2++){
				if(row & test){
					float oldX = tempx;
					do{
						tempx+=size;
						test/=2;
						x2++;
					}while(row & test);
					drawBox(oldX, tempy, tempx, tempy+size);
				}
				tempx+=size;
				test/=2;
			}
			tempy -= size;
		}
	}
}
