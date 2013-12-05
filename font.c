#include <SDL2/SDL2_gfxPrimitives_font.h>
#include <SDL2/SDL.h>
#include "gfx.h"

void drawText(Uint32 *screen, int x, int y, Uint32 color, float size, char* string){
	int j = -1;
	int i = -1;
	int y2;
	int x2;
	register Uint8 row;
	register Uint8 test;
	float tempx, tempy;
	int boxX, boxY;
	while(string[++j] != '\0'){
		i++;
		if(string[j] == '\n'){
			i = -1;
			y+=size*9;
			continue;
		}
		tempy = y;
		for(y2 = 0; y2<8; y2++){
			row = gfxPrimitivesFontdata[8*string[j]+y2];
			test = 128;
			tempx = x+i*9*size;
			for(x2 = 0; x2<8; x2++){
				tempx+=size;
				if(!(row & test)){
					test/=2;
					continue;
				}
				boxX = (int)(tempx-size);
				boxY = (int)tempy;
				boxColor(screen, boxX, boxY, (int)tempx-boxX-1, (int)(tempy+size)-boxY, color-1);
				test/=2;
			}
			tempy += size;
		}
	}
}
