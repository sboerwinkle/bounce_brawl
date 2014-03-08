#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include "gui.h"
#include "gfx.h"

static GLuint uniColorId, vbo;

int width2, height2;

void initGfx(){
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	const GLchar* vertexPrg =
"#version 120\n"
"attribute vec2 pos;"
//"in vec3 color;"
//"out vec3 fragColor;"
"void main(){"
//"fragColor = color;"
"gl_Position = vec4(pos, 0.0, 1.0);"
"}";
	const GLchar* fragmentPrg =
"#version 120\n"
//"in vec3 fragColor;"
"uniform vec3 uniColor;"
//"out vec4 outColor;"
"void main(){"
"gl_FragColor = vec4(uniColor, 1.0);"
"}";
	GLuint vertexPrgId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexPrgId, 1, &vertexPrg, NULL);
	glCompileShader(vertexPrgId);
	GLuint fragmentPrgId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentPrgId, 1, &fragmentPrg, NULL);
	glCompileShader(fragmentPrgId);
	GLuint prgId = glCreateProgram();
	glAttachShader(prgId, vertexPrgId);
	glAttachShader(prgId, fragmentPrgId);
//	glBindFragDataLocation(prgId, 0, "outColor");
	glLinkProgram(prgId);
	glUseProgram(prgId);

	GLint status;
	glGetShaderiv(vertexPrgId, GL_COMPILE_STATUS, &status);
	if(status!=GL_TRUE){
		puts("OMG AAAAAAAAAAAAAAA");
		char buffer[512];
		glGetShaderInfoLog(vertexPrgId, 512, NULL, buffer);
		puts(buffer);
	}
	glGetShaderiv(fragmentPrgId, GL_COMPILE_STATUS, &status);
	if(status!=GL_TRUE){
		puts("OMG AAAAAAAAHHHHHHH");
		char buffer[512];
		glGetShaderInfoLog(fragmentPrgId, 512, NULL, buffer);
		puts(buffer);
	}

	GLint posAttrib = glGetAttribLocation(prgId, "pos");
//	GLint colorAttrib = glGetAttribLocation(prgId, "color");
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
//	glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));
	glEnableVertexAttribArray(posAttrib);
//	glEnableVertexAttribArray(colorAttrib);
	uniColorId = glGetUniformLocation(prgId, "uniColor");

	glLineWidth(2);
}

void setColorFromHue(uint16_t hue){
	if(cheats & CHEAT_COLORS) return;
	float r, g, b;
	if(hue < 64){
		r=0;
		b=1;
		g=hue/64.0;
	}else if(hue < 128){
		r=0;
		g=1;
		b=1.0 - (hue-64)/64.0;
	}else if(hue < 192){
		g=1;
		b=0;
		r=(hue-128)/64.0;
	}else if(hue < 256){
		r=1;
		b=0;
		g=1-(hue-192)/64.0;
	}else if(hue < 320){
		r=1;
		g=0;
		b=(hue-256)/64.0;
	}else{
		g=0;
		b=1;
		r=1-(hue-320)/64.0;
	}
	glUniform3f(uniColorId, r, g, b);
}

uint32_t getColorFromHue(uint16_t hue){
	if(cheats & CHEAT_COLORS) return 0xFFFFFFFF;
	float r, g, b;
	if(hue < 64){
		r=0;
		b=1;
		g=hue/64.0;
	}else if(hue < 128){
		r=0;
		g=1;
		b=1.0 - (hue-64)/64.0;
	}else if(hue < 192){
		g=1;
		b=0;
		r=(hue-128)/64.0;
	}else if(hue < 256){
		r=1;
		b=0;
		g=1-(hue-192)/64.0;
	}else if(hue < 320){
		r=1;
		g=0;
		b=(hue-256)/64.0;
	}else{
		g=0;
		b=1;
		r=1-(hue-320)/64.0;
	}
	return 0x01000000*(int)(r*0xFF) + 0x010000*(int)(g*0xFF) + 0x0100*(int)(b*0xFF) + 0xFF;
}

void setColorWhite(){glUniform3f(uniColorId, 1.0, 1.0, 1.0);}

void setColorFromHex(uint32_t color){
	if(cheats & CHEAT_COLORS) return;
	glUniform3f(uniColorId, (float)(color&0xFF000000)/0xFF000000, (float)(color&0xFF0000)/0xFF0000, (float)(color&0xFF00)/0xFF00);
}

void drawBox(float x1, float y1, float x2, float y2){
	float points[]={
		x1, y1,
		x2, y1,
		x2, y2,
		x1, y2};
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STREAM_DRAW);
	glDrawArrays(GL_QUADS, 0, 4);
	glInvalidateBufferData(vbo);
}

void drawRectangle(float x1, float y1, float x2, float y2){
	float points[]={
		x1, y1,
		x2, y1,
		x2, y2,
		x1, y2};
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STREAM_DRAW);
	glDrawArrays(GL_LINE_LOOP, 0, 4);
	glInvalidateBufferData(vbo);
}

void drawLine(float x1, float y1, float x2, float y2){
	float points[]={x1, y1, x2, y2};
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STREAM_DRAW);
	glDrawArrays(GL_LINES, 0, 2);
	glInvalidateBufferData(vbo);
}

void drawCircle(float cx, float cy, float r){
	int numSegments;
	if(r<0.004) numSegments = 4;
	else numSegments = (int)(60*sqrtf(r));
	float* points = malloc(numSegments*2*sizeof(float));
	float* current = points;
	float t = 2*M_PI/numSegments;//'t' is for theta
	float myCos = cosf(t);
	float mySin = sinf(t);
	float x = r;
	float y = 0;
	int count = numSegments;
	for(; count > 0; count--){
		*(current++) = cx+x;
		*(current++) = cy+y;
		t = x;//'t' is for temporary storage
		x = myCos*x + mySin*y;
		y = myCos*y - mySin*t;
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*numSegments, points, GL_STREAM_DRAW);
	glDrawArrays(GL_LINE_LOOP, 0, numSegments);
	glInvalidateBufferData(vbo);
	free(points);
}
