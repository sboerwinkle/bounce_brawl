//See gfx.h -Simon

/* 

SDL2_gfxPrimitives.c: graphics primitives for SDL2 renderers

Copyright (C) 2012  Andreas Schiffler

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.

Andreas Schiffler -- aschiffler at ferzkopp dot net

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "gfx.h"
//#include "SDL2_rotozoom.h"
//#include "SDL2_gfxPrimitives_font.h"
#define pitch 750
#define imgHeight 750

/* ---- Structures */

/*!
\brief The structure passed to the internal Bresenham iterator.
*/
/*typedef struct {
	Sint16 x, y;
	int dx, dy, s1, s2, swapdir, error;
	Uint32 count;
} SDL2_gfxBresenhamIterator;
*/
/*!
\brief The structure passed to the internal Murphy iterator.
*/
/*typedef struct {
	SDL_Renderer *renderer;
	int u, v;		// delta x , delta y
	int ku, kt, kv, kd;	// loop constants
	int oct2;
	int quad4;
	Sint16 last1x, last1y, last2x, last2y, first1x, first1y, first2x, first2y, tempx, tempy;
} SDL2_gfxMurphyIterator;*/

/* ---- Pixel */

/* ---- Hline */

/*!
\brief Draw horizontal line with blending.

\param renderer The renderer to draw on.
\param x1 X coordinate of the first point (i.e. left) of the line.
\param x2 X coordinate of the second point (i.e. right) of the line.
\param y Y coordinate of the points of the line.
\param color The color value of the line to draw (0xRRGGBBAA). 

\returns Returns 0 on success, -1 on failure.
*/
void hlineColor(Uint32 *screen, Sint16 x1, Sint16 width, Sint16 y, Uint32 color)
{
	if(y < 0 || y >= imgHeight || x1 >= pitch) return;
	if(x1 < 0){
		if(width < -x1) return;
		width += x1;
		x1 = 0;
	}
	if(x1 + width >= pitch) width = pitch - 1 - x1;
	register int ix = y*pitch + x1;
	int max = ix+width;
	for(; ix <= max; ix++) screen[ix] = color;
}

/* ---- Vline */

/*!
\brief Draw vertical line with blending.

\param renderer The renderer to draw on.
\param x X coordinate of the points of the line.
\param y1 Y coordinate of the first point (i.e. top) of the line.
\param y2 Y coordinate of the second point (i.e. bottom) of the line.
\param color The color value of the line to draw (0xRRGGBBAA). 

\returns Returns 0 on success, -1 on failure.
*/
void vlineColor(Uint32 *screen, Sint16 x, Sint16 y1, Sint16 height, Uint32 color)
{
	if(x < 0 || x >= pitch || y1 >= imgHeight) return;
	if(y1 < 0){
		if(height < -y1) return;
		height += y1;
		y1 = 0;
	}
	if(y1 + height >= imgHeight) height = imgHeight - 1 - y1;
	register int ix = x+y1*pitch;
	int max = ix+height*pitch;
	for(; ix <= max; ix+= pitch) screen[ix] = color;
}


/* ---- Rectangle */

/*!
\brief Draw rectangle with blending.

\param renderer The renderer to draw on.
\param x1 X coordinate of the first point (i.e. top right) of the rectangle.
\param y1 Y coordinate of the first point (i.e. top right) of the rectangle.
\param x2 X coordinate of the second point (i.e. bottom left) of the rectangle.
\param y2 Y coordinate of the second point (i.e. bottom left) of the rectangle.
\param color The color value of the rectangle to draw (0xRRGGBBAA). 

\returns Returns 0 on success, -1 on failure.
*/
void boxColor(Uint32 *screen, Sint16 x1, Sint16 y1, Sint16 width, Sint16 height, Uint32 color)
{
	if(x1 >= pitch || y1 >= imgHeight) return;
	if(x1 < 0){
		if(width < -x1) return;
		width += x1;
		x1 = 0;
	}
	if(y1 < 0){
		if(height < -y1) return;
		height += y1;
		y1 = 0;
	}
	if(x1 + width >= pitch) width = pitch - 1 - x1;
	if(y1 + height >= imgHeight) height = imgHeight - 1 - y1;
	int y = y1*pitch+x1;
	int ymax = pitch*height + y;
	register int ix;
	int max;
	for(; y <= ymax; y += pitch){
		ix = y;
		max = ix+width;
		for(; ix <= max; ix++) screen[ix] = color;
	}
}

/* ---- Rounded Rectangle */

/* ---- Box */

/*!
\brief Draw box (filled rectangle) with blending.

\param renderer The renderer to draw on.
\param x1 X coordinate of the first point (i.e. top right) of the box.
\param y1 Y coordinate of the first point (i.e. top right) of the box.
\param x2 X coordinate of the second point (i.e. bottom left) of the box.
\param y2 Y coordinate of the second point (i.e. bottom left) of the box.
\param color The color value of the box to draw (0xRRGGBBAA). 

\returns Returns 0 on success, -1 on failure.
*/
void rectangleColor(Uint32 *screen, Sint16 x1, Sint16 y1, Sint16 width, Sint16 height, Uint32 color)
{
	hlineColor(screen, x1, width, y1, color);
	hlineColor(screen, x1, width, y1+height, color);
	vlineColor(screen, x1, y1+1, height-2, color);
	vlineColor(screen, x1+width, y1+1, height-2, color);
}

/* ---- Line */

/*!
\brief Draw line with alpha blending.

\param renderer The renderer to draw on.
\param x1 X coordinate of the first point of the line.
\param y1 Y coordinate of the first point of the line.
\param x2 X coordinate of the second point of the line.
\param y2 Y coordinate of the seond point of the line.
\param color The color value of the line to draw (0xRRGGBBAA). 

\returns Returns 0 on success, -1 on failure.
*/
void lineColor(Uint32 *screen, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color)
{
	if(y1 == y2){
		if(x2 >= x1)
			hlineColor(screen, x1, x2-x1, y1, color);
		else
			hlineColor(screen, x2, x1-x2, y1, color);
		return;
	}
	if(x1 == x2){
		if(y2 >= y1)
			vlineColor(screen, x1, y1, y2-y1, color);
		else
			vlineColor(screen, x1, y2, y1-y2, color);
		return;
	}
	Sint16 dx = x2-x1;
	Sint16 dy = y2-y1;
	if(dx > 0){
		if(x1 >= pitch || x2 < 0) return;
	}else{
		if(x2 >= pitch || x1 < 0) return;
	}
	if(dy > 0){
		if(y1 >= imgHeight || y2 < 0) return;
	}else{
		if(y2 >= imgHeight || y1 < 0) return;
	}
	Sint16 x, y;
	double step;
	Sint8 otherStep;
	Sint8 flag = 0;		//Used to indicate that the line will run out of bounds on its minor axis
	double breakPoint;
	int breakPointRound;
	if(abs(dx) > abs(dy)){
		if(dx < 0){
			x = x2;
			y = y2;
			x2 = x1;
			y2 = y1;
			dx = -dx;
			dy = -dy;
		}else{
			x = x1;
			y = y1;
		}
		if(dy>0){
			otherStep = 1;
			step = (double)dx/dy;
			if(y2>=imgHeight){
				y2 = imgHeight;
				flag = 1;
			}
		}else{
			otherStep = -1;
			step = (double)-dx/dy;
			if(y2<0){
				y2 = -1;
				flag = 1;
			}
		}
		for(breakPoint = x + 0.5 + step/2; y != y2; breakPoint += step){
			breakPointRound = (int)breakPoint;
			hlineColor(screen, x, breakPointRound-x, y, color);
			if(breakPointRound >= pitch) return;
			x = breakPointRound;
			y += otherStep;

		}
		if(flag) return;
		hlineColor(screen, x, x2-x, y, color);
	}else{
		if(dy < 0){
			x = x2;
			y = y2;
			x2 = x1;
			y2 = y1;
			dx = -dx;
			dy = -dy;
		}else{
			x = x1;
			y = y1;
		}
		if(dx>0){
			otherStep = 1;
			step = (double)dy/dx;
			if(x2 >= pitch){
				x2 = pitch;
				flag = 1;
			}
		}else{
			otherStep = -1;
			step = (double)-dy/dx;
			if(x2 < 0){
				x2 = -1;
				flag = 1;
			}
		}
		for(breakPoint = y + 0.5 + step/2; x != x2; breakPoint += step){
			breakPointRound = (int)breakPoint;
			vlineColor(screen, x, y, breakPointRound-y, color);
			if(breakPointRound >= imgHeight) return;
			y = breakPointRound;
			x += otherStep;
		}
		if(flag) return;
		vlineColor(screen, x, y, y2-y, color);
	}
}

/* ---- AA Line */

//#define AAlevels 256
//#define AAbits 8

/* ----- Circle */

/* ----- Arc */

/* ----- AA Circle */

/* ----- Filled Circle */

/* ----- Ellipse */

/*!
\brief Draw ellipse with blending.

\param renderer The renderer to draw on.
\param x X coordinate of the center of the ellipse.
\param y Y coordinate of the center of the ellipse.
\param rx Horizontal radius in pixels of the ellipse.
\param ry Vertical radius in pixels of the ellipse.
\param r The red value of the ellipse to draw. 
\param g The green value of the ellipse to draw. 
\param b The blue value of the ellipse to draw. 
\param a The alpha value of the ellipse to draw.

\returns Returns 0 on success, -1 on failure.
*/
void ellipseColor(Uint32 *screen, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color)
{
	if(x + rx < 0 || x - rx >= pitch || y + ry < 0 || y - ry >= imgHeight) return;

	int vertBound, horizBound;
	if(y < 0) vertBound = -y;
	else if(y >= imgHeight) vertBound = y - imgHeight + 1;
	else vertBound = 0;
	if(x < 0) horizBound = -x;
	else if(x >= pitch) horizBound = x - pitch + 1;
	else horizBound = 0;

	int ix, iy;
	int h, i, j, k;
	int oh, oi, oj, ok;
	int xmh, xph, ypk, ymk;
	int xmi, xpi, ymj, ypj;
	int xmj, xpj, ymi, ypi;
	int xmk, xpk, ymh, yph;

	int simon;	//This variable is just used so I don't have to convert the y component to an array index as often. You'll see when it's used. Named after your fearless leader.

	/*
	* Init vars 
	*/
	oh = oi = oj = ok = 0xFFFF;

	/*
	* Draw 
	*/
	if (rx > ry) {
		ix = 0;
		iy = rx * 64;

		do {
			h = (ix + 32) >> 6;
			i = (iy + 32) >> 6;
			j = (h * ry) / rx;
			k = (i * ry) / rx;

			if (((ok != k) && (oj != k)) || ((oj != j) && (ok != j)) || (k != j)) {
				if(h >= horizBound && k >= vertBound){
					xph = x + h;
					xmh = x - h;
					if (k > 0) {
						ypk = y + k;
						ymk = y - k;
						if(ypk < imgHeight){
							simon = pitch*ypk;
							if(xmh >= 0) screen[simon+xmh]=color;
							if(xph < pitch) screen[simon+xph]=color;
						}
						if(ymk >= 0){
							simon = pitch*ymk;
							if(xmh >= 0) screen[simon+xmh]=color;
							if(xph < pitch) screen[simon+xph]=color;
						}
	//					result |= pixel(renderer, xmh, ypk);
	//					result |= pixel(renderer, xph, ypk);
	//					result |= pixel(renderer, xmh, ymk);
	//					result |= pixel(renderer, xph, ymk);
					} else {
						simon = pitch*y;
						if(xmh >= 0) screen[simon+xmh]=color;
						if(xph < pitch) screen[simon+xph]=color;
	//					result |= pixel(renderer, xmh, y);
	//					result |= pixel(renderer, xph, y);
					}
				}
				ok = k;
				if(i >= horizBound && j >= vertBound){
					xpi = x + i;
					xmi = x - i;
					if (j > 0) {
						ypj = y + j;
						ymj = y - j;
						if(ypj < imgHeight){
							simon = pitch*ypj;
							if(xmi >= 0) screen[simon+xmi]=color;
							if(xpi < pitch) screen[simon+xpi]=color;
						}
						if(ymj >= 0){
							simon = pitch*ymj;
							if(xmi >= 0) screen[simon+xmi]=color;
							if(xpi < pitch) screen[simon+xpi]=color;
						}
	//					result |= pixel(renderer, xmi, ypj);
	//					result |= pixel(renderer, xpi, ypj);
	//					result |= pixel(renderer, xmi, ymj);
	//					result |= pixel(renderer, xpi, ymj);
					} else {
						simon = pitch*y;
						if(xmi >= 0) screen[simon+xmi]=color;
						if(xpi < pitch) screen[simon+xpi]=color;
	//					result |= pixel(renderer, xmi, y);
	//					result |= pixel(renderer, xpi, y);
					}
				}
				oj = j;
			}

			ix = ix + iy / rx;
			iy = iy - ix / rx;

		} while (i > h);
	} else {
		ix = 0;
		iy = ry * 64;

		do {
			h = (ix + 32) >> 6;
			i = (iy + 32) >> 6;
			j = (h * rx) / ry;
			k = (i * rx) / ry;

			if (((oi != i) && (oh != i)) || ((oh != h) && (oi != h) && (i != h))) {
				if(j >= horizBound && i >= vertBound){
					xmj = x - j;
					xpj = x + j;
					if (i > 0) {
						ypi = y + i;
						ymi = y - i;
						if(ypi < imgHeight){
							simon = pitch*ypi;
							if(xmj >= 0) screen[simon+xmj]=color;
							if(xpj < pitch) screen[simon+xpj]=color;
						}
						if(ymi >= 0){
							simon = pitch*ymi;
							if(xmj >= 0) screen[simon+xmj]=color;
							if(xpj < pitch) screen[simon+xpj]=color;
						}
	//					result |= pixel(renderer, xmj, ypi);
	//					result |= pixel(renderer, xpj, ypi);
	//					result |= pixel(renderer, xmj, ymi);
	//					result |= pixel(renderer, xpj, ymi);
					} else {
						simon = pitch*y;
						if(xmj >= 0) screen[simon+xmj]=color;
						if(xpj < pitch) screen[simon+xpj]=color;
	//					result |= pixel(renderer, xmj, y);
	//					result |= pixel(renderer, xpj, y);
					}
				}
				oi = i;
				if(k >= horizBound && h >= vertBound){
					xmk = x - k;
					xpk = x + k;
					if (h > 0) {
						yph = y + h;
						ymh = y - h;
						if(yph < imgHeight){
							simon = pitch*yph;
							if(xmk >= 0) screen[simon+xmk]=color;
							if(xpk < pitch) screen[simon+xpk]=color;
						}
						if(ymh >= 0){
							simon = pitch*ymh;
							if(xmk >= 0) screen[simon+xmk]=color;
							if(xpk < pitch) screen[simon+xpk]=color;
						}
	//					result |= pixel(renderer, xmk, yph);
	//					result |= pixel(renderer, xpk, yph);
	//					result |= pixel(renderer, xmk, ymh);
	//					result |= pixel(renderer, xpk, ymh);
					} else {
						simon = pitch*y;
						if(xmk >= 0) screen[simon+xmk]=color;
						if(xpk < pitch) screen[simon+xpk]=color;
	//					result |= pixel(renderer, xmk, y);
	//					result |= pixel(renderer, xpk, y);
					}
				}
				oh = h;
			}

			ix = ix + iy / ry;
			iy = iy - ix / ry;

		} while (i > h);
	}
}

/* ----- AA Ellipse */

/* Windows targets do not have lrint, so provide a local inline version */
#if defined(_MSC_VER)
/* Detect 64bit and use intrinsic version */
#ifdef _M_X64
#include <emmintrin.h>
static __inline long 
	lrint(float f) 
{
	return _mm_cvtss_si32(_mm_load_ss(&f));
}
#elif defined(_M_IX86)
__inline long int
	lrint (double flt)
{	
	int intgr;
	_asm
	{
		fld flt
			fistp intgr
	};
	return intgr;
}
#elif defined(_M_ARM)
#include <armintr.h>
#pragma warning(push)
#pragma warning(disable: 4716)
__declspec(naked) long int
	lrint (double flt)
{
	__emit(0xEC410B10); // fmdrr  d0, r0, r1
	__emit(0xEEBD0B40); // ftosid s0, d0
	__emit(0xEE100A10); // fmrs   r0, s0
	__emit(0xE12FFF1E); // bx     lr
}
#pragma warning(pop)
#else
#error lrint needed for MSVC on non X86/AMD64/ARM targets.
#endif
#endif


//ATTENTION. The below function has not been tested yet. It might conceivably draw out of bounds or just not work. That is all.

/* ---- Filled Ellipse */

/*!
\brief Draw filled ellipse with blending. Untested.

\param renderer The renderer to draw on.
\param x X coordinate of the center of the filled ellipse.
\param y Y coordinate of the center of the filled ellipse.
\param rx Horizontal radius in pixels of the filled ellipse.
\param ry Vertical radius in pixels of the filled ellipse.
\param r The red value of the filled ellipse to draw. 
\param g The green value of the filled ellipse to draw. 
\param b The blue value of the filled ellipse to draw. 
\param a The alpha value of the filled ellipse to draw.

\returns Returns 0 on success, -1 on failure.
*/
void filledEllipseRGBA(Uint32 *screen, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color)
{
	if(x + rx < 0 || x - rx >= pitch || y + ry < 0 || y - ry >= imgHeight) return;
	int ix, iy;
	int h, i, j, k;
	int oh, oi, oj, ok;
	int xmh, xph;
	int xmi, xpi;
	int xmj, xpj;
	int xmk, xpk;

	/*
	* Init vars 
	*/
	oh = oi = oj = ok = 0xFFFF;

	/*
	* Draw 
	*/
	if (rx > ry) {
		ix = 0;
		iy = rx * 64;

		do {
			h = (ix + 32) >> 6;
			i = (iy + 32) >> 6;
			j = (h * ry) / rx;
			k = (i * ry) / rx;

			if ((ok != k) && (oj != k)) {
				xph = 2 * h;
				xmh = x - h;
				if (k > 0) {
					hlineColor(screen, xmh, xph, y + k, color);
					hlineColor(screen, xmh, xph, y - k, color);
				} else {
					hlineColor(screen, xmh, xph, y, color);
				}
				ok = k;
			}
			if ((oj != j) && (ok != j) && (k != j)) {
				xmi = x - i;
				xpi = 2 * i;
				if (j > 0) {
					hlineColor(screen, xmi, xpi, y + j, color);
					hlineColor(screen, xmi, xpi, y - j, color);
				} else {
					hlineColor(screen, xmi, xpi, y, color);
				}
				oj = j;
			}

			ix = ix + iy / rx;
			iy = iy - ix / rx;

		} while (i > h);
	} else {
		ix = 0;
		iy = ry * 64;

		do {
			h = (ix + 32) >> 6;
			i = (iy + 32) >> 6;
			j = (h * rx) / ry;
			k = (i * rx) / ry;

			if ((oi != i) && (oh != i)) {
				xmj = x - j;
				xpj = 2 * j;
				if (i > 0) {
					hlineColor(screen, xmj, xpj, y + i, color);
					hlineColor(screen, xmj, xpj, y - i, color);
				} else {
					hlineColor(screen, xmj, xpj, y, color);
				}
				oi = i;
			}
			if ((oh != h) && (oi != h) && (i != h)) {
				xmk = x - k;
				xpk = 2 * k;
				if (h > 0) {
					hlineColor(screen, xmk, xpk, y + h, color);
					hlineColor(screen, xmk, xpk, y - h, color);
				} else {
					hlineColor(screen, xmk, xpk, y, color);
				}
				oh = h;
			}

			ix = ix + iy / ry;
			iy = iy - ix / ry;

		} while (i > h);
	}
}

/* ----- Pie */
/* ------ Trigon */
/* ---- Polygon */
/* ---- AA-Polygon */
/* ---- Filled Polygon */
/* ---- Textured Polygon */
/* ---- Character */
