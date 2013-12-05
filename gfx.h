/* So, I read the below thing. Sounds like I need to say:
 * THIS IS AN ALTERED VERSION
 * I basically just took dude man's logic and applied it to a different drawing surface.
 * -Simon Boerwinkle
 */

/* 

SDL2_gfxPrimitives.h: graphics primitives for SDL

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

#ifndef gfx_h
#define gfx_h

#include <math.h>
#ifndef M_PI
#define M_PI	3.1415926535897932384626433832795
#endif

	/* ---- Function Prototypes */
/*
#ifdef _MSC_VER
#  if defined(DLL_EXPORT) && !defined(LIBSDL2_GFX_DLL_IMPORT)
#    define extern __declspec(dllexport)
#  else
#    ifdef LIBSDL2_GFX_DLL_IMPORT
#      define extern __declspec(dllimport)
#    endif
#  endif
#endif
#ifndef extern
#  define extern extern
#endif
*/
	/* Note: all ___Color routines expect the color to be in format 0xRRGGBBAA */
	/* Pixel */

	/* Horizontal line */
	extern void hlineColor(Uint32 *screen, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color);

	/* Vertical line */
	extern void vlineColor(Uint32 *screen, Sint16 x, Sint16 y1, Sint16 y2, Uint32 color);

	/* Rectangle */
	extern void rectangleColor(Uint32 *screen, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);

	/* Rounded-Corner Rectangle */

	/* Filled rectangle (Box) */
	extern void boxColor(Uint32 *screen, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);

	/* Rounded-Corner Filled rectangle (Box) */

	/* Line */
	extern void lineColor(Uint32 *screen, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);

	/* AA Line */
	/* Thick Line */
	/* Circle */
	/* Arc */
	/* AA Circle */
	/* Filled Circle */

	/* Ellipse */
	extern void ellipseColor(Uint32 *screen, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color);

	/* AA Ellipse */

	/* Filled Ellipse */
	extern void filledEllipseColor(Uint32 *screen, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color);

	/* Pie */
	/* Filled Pie */
	/* Trigon */
	/* AA-Trigon */
	/* Filled Trigon */

	/* Polygon */
//	extern void polygonColor(Uint32 *screen, const Sint16 * vx, const Sint16 * vy, int n, Uint32 color);

	/* AA-Polygon */

	/* Filled Polygon */
//	extern void filledPolygonColor(Uint32 *screen, const Sint16 * vx, const Sint16 * vy, int n, Uint32 color);

	/* Textured Polygon */
	/* Bezier */
	/* Characters/Strings */
#endif //For the gfx_h define
