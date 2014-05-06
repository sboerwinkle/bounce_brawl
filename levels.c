#include <stdlib.h>
#include <stdio.h>
#include "structs.h"
#include "field.h"
#include "task.h"
#include "node.h"
#include "gui.h"
#include <math.h>

static int dist(node* a, node* b){
	int dx = (int)(a->x - b->x);
	int dy = (int)(a->y - b->y);
	return (int)sqrt(dx*dx + dy*dy);
}
static double preciseDist(int a, int b){
	double dx = (nodes[a].x-nodes[b].x) + (nodes[a].px-nodes[b].px);
	double dy = (nodes[a].y-nodes[b].y) + (nodes[a].py-nodes[b].py);
	return sqrt(dx*dx + dy*dy);
}
static void connectNodes(int a, int b, double friction, double tol, double str){
	double dx = (nodes[a].x-nodes[b].x) + (nodes[a].px-nodes[b].px);
	double dy = (nodes[a].y-nodes[b].y) + (nodes[a].py-nodes[b].py);
	newConnection(a, createConnection(a), b, friction, sqrt(dx*dx+dy*dy), tol, str);
}

static void addScores(){
	double scoreInc;
	if(players>1) scoreInc = 602.0/(players-1);
	else scoreInc=0;
	int i = 0;
	for(; i < players; i++){
		taskscoreaddLong(i, 20+i*scoreInc, 20);
	}
}
#define sqrt3 1.732050808
#define H 1
//Makes arrays created for this method easier to visualize, especially w/ syntax highlighting
static int addHex(double x, double y, int width, int height, int* map, double fric, double spacing, double tolerance, double str, double size, double mass){
	int ret = -1;
	double placeX;
	double placeY = y;
	int tmp;
	int i, j;
	for(j = 0; j < height; j++){
		placeX = x + j*spacing/2;
		for(i = 0; i < width; i++){
			if(map[j*width + i]){
				tmp = addNode();
				if(ret==-1) ret=tmp;
				newNode(tmp, (int)placeX, (int)placeY, size, mass, 0);
				map[j*width + i] = tmp+1; // index is stored so later nodes can connect to it
				//Initialize connections
				int subI, subJ;
				double mySpacing = spacing;
				for(subI = i-1; subI >= 0; subI--){
					if(map[j*width+subI]){
						newConnection(tmp, createConnection(tmp), map[j*width+subI]-1, fric, mySpacing, tolerance, str);
						break;
					}
					mySpacing += spacing;
				}
				mySpacing = spacing;
				for(subJ = j-1; subJ >= 0; subJ--){
					if(map[subJ*width+i]){
						newConnection(tmp, createConnection(tmp), map[subJ*width+i]-1, fric, mySpacing, tolerance, str);
						break;
					}
					mySpacing += spacing;
				}
				mySpacing = spacing;
				subI = i+1;
				for(subJ = j-1; subJ >= 0 && subI < width; subJ--){
					if(map[subJ*width+subI]){
						newConnection(tmp, createConnection(tmp), map[subJ*width+subI]-1, fric, mySpacing, tolerance, str);
						break;
					}
					subI++;
					mySpacing += spacing;
				}
			}
			placeX += spacing;
		}
		placeY += spacing*sqrt3/2;
	}
	return ret;
}
static void addBridge(int ix1, int ix2, int numNodes, double height, double size, double mass, double fric, double tol, double str){//'height' is a fraction of the total distance.
	double stepx = (nodes[ix2].x-nodes[ix1].x+nodes[ix2].px-nodes[ix1].px)/numNodes;
	double stepy = (nodes[ix2].y-nodes[ix1].y+nodes[ix2].py-nodes[ix1].py)/numNodes;
	double stepDist = sqrt(stepx*stepx + stepy*stepy);
	double x = nodes[ix1].x+nodes[ix1].px;
	double y = nodes[ix1].y+nodes[ix1].py;
	int ix = addNode();
	int i = 1;
	height *= numNodes;
	newNode(ix, x + stepx*numNodes/2 + stepy*height, y + stepy*numNodes/2 - stepx*height, size, mass*3, 3);
	for(; i < numNodes; i++){
		newNode(addNode(), x+=stepx, y+=stepy, size, mass, 1);
		newConnection(ix+i, 0, ix+i+1, fric, stepDist, tol, str);
	}
	nodes[ix+numNodes-1].connections[0].id = ix2;
	newConnection(ix+1, createConnection(ix+1), ix1, fric, stepDist, tol, str);
	newConnection(ix, 0, ix1, fric, preciseDist(ix, ix1), tol, str*3);
	newConnection(ix, 1, ix2, fric, preciseDist(ix, ix2), tol, str*3);
	newConnection(ix, 2, ix+numNodes/2, fric, preciseDist(ix, ix+numNodes/2), tol, str*3);
}
static void addLoop(int centerX, int centerY, double radius, int num, double theta, double size, double mass, double fric, double tol, double str){
	double angleInc = M_PI*2/num;
	int ix = 0;
	double xpart, ypart;
	double len = 2*radius*sin(angleInc/2);
	int i = 0;
	for(; i < num; i++){
		xpart = radius*cos(theta);
		ypart = radius*sin(theta);
		ix = addNode();
		newNodeLong(ix, (int)xpart+centerX, (int)ypart+centerY, xpart-(int)xpart, ypart-(int)ypart, 0, 0, size, mass, 1);
		newConnection(ix, 0, ix+1, fric, len, tol, str);
		theta += angleInc;
	}
	nodes[ix].connections[0].id = ix-num+1;
}
static void addBlock(int x, int y, int width, int height, double fric, int spacing, double heightSpacing, double tolerance, double str, double size, double mass){
	int ix;
	char indented = 1;
	int X;
	double Y = y;
	register int i = 0;
	register int j;
	for(; i < height-1; i++){
		X = indented?x+spacing/2:x;
		y = (int)Y;
		ix = addNode();
		newNode(ix, X, y, size, mass, (indented&&width>1)?3:2);
		int connection = 0;
		if(!(indented && width == 1))
			newConnection(ix, connection++, ix+1, fric, spacing, tolerance, str);
		if(indented)
			newConnection(ix, connection++, ix+width, fric, spacing, tolerance, str);
		newConnection(ix, connection++, ix+width+1, fric, spacing, tolerance, str);
		X += spacing;
		for(j = indented?2:1; j < width; j++){
			ix = addNode();
			newNode(ix, X, y, size, mass, 3);
			newConnection(ix, 0, ix+1, fric, spacing, tolerance, str);
			newConnection(ix, 1, ix+width, fric, spacing, tolerance, str);
			newConnection(ix, 2, ix+width+1, fric, spacing, tolerance, str);
			X += spacing;
		}
		if(width > 1 || !indented){
			ix = addNode();
			newNode(ix, X, y, size, mass, indented?2:1);
			newConnection(ix, 0, ix+width, fric, spacing, tolerance, str);
			if(indented)
				newConnection(ix, 1, ix+width+1, fric, spacing, tolerance, str);
			
		}
		indented = !indented;
		Y+=heightSpacing;
	}
	X = indented?x+spacing/2:x;
	y = (int)Y;
	for(j = indented?1:0; j < width; j++){
		ix = addNode();
		newNode(ix, X, y, size, mass, 1);
		newConnection(ix, 0, ix+1, fric, spacing, tolerance, str);
		X += spacing;
	}
	newNode(addNode(), X, y, size, mass, 0);
}
#define BCS 12.5
#define BMCS 17.5
#define BNS 9
#define BRNS 11
#define BMNS 10
#define BNM 20
#define BMNM 30
#define BCF 0.6
#define BNUM 12
#define BHT 75
static void addBuilding(double x, double y, int stories){
	int ix = addNode();
	int inc = stories%2?-80:80;
	if(stories%2) x += 80*3;
	newNode(ix, x, y, BMNS, BMNM, stories?3:1);
	newNode(addNode(), x+inc, y, BMNS, BMNM, stories?3:1);
	newNode(addNode(), x+2*inc, y, BMNS, BMNM, stories?3:1);
	newNode(addNode(), x+3*inc, y, BMNS, BMNM, stories?1:0);
	newConnection(ix, 0, ix+1, .9, 80, 7, BMCS);
	newConnection(ix+1, 0, ix+2, .9, 80, 7, BMCS);
	newConnection(ix+2, 0, ix+3, .9, 80, 7, BMCS);

	newNode(addNode(), x+inc*1.25, y, BNS, BNM, 2);
	newNode(addNode(), x+inc*1.50, y, BNS, BNM, 1);
	newNode(addNode(), x+inc*1.75, y, BNS, BNM, 1);
	newConnection(ix+4, 0, ix+1, BCF, 20, 4, BCS);
	newConnection(ix+4, 1, ix+5, BCF, 20, 4, BCS);
	newConnection(ix+5, 0, ix+6, BCF, 20, 4, BCS);
	newConnection(ix+6, 0, ix+2, BCF, 20, 4, BCS);

//	addToolGrab(ix+1);

	double diag = sqrt(BHT*BHT + 80*80);
	double diagTol = diag*7/40;

		newNode(addNode(), x+inc*2.25, y, BNS, BNM, 2);
		newNode(addNode(), x+inc*2.50, y, BNS, BNM, 1);
		newNode(addNode(), x+inc*2.75, y, BNS, BNM, 1);
		newConnection(ix+7, 0, ix+2, BCF, 20, 4, BCS);
		newConnection(ix+7, 1, ix+8, BCF, 20, 4, BCS);
		newConnection(ix+8, 0, ix+9, BCF, 20, 4, BCS);
		newConnection(ix+9, 0, ix+3, BCF, 20, 4, BCS);

	newNode(addNode(), x+inc*1.0/3, y, BRNS, BNM/4.0, 2);
	newNode(addNode(), x+inc*2.0/3, y, BRNS, BNM/4.0, 1);
	newConnection(ix+10, 1, ix, BCF, 80.0/3, 7, BCS/3);
	newConnection(ix+10, 0, ix+11, BCF, 80.0/3, 7, BCS/3);
	newConnectionLong(ix+11, 0, ix+1, 0.2, 80.0/3, (80.0*1/2+80.0*.75)/2, 40, BCS/35);
	addToolToggle(ix+11);

	if(stories){
		newConnection(ix,   1, ix+BNUM+3, .9, BHT, 7, BMCS);
		newConnection(ix,   2, ix+BNUM+2, .9, diag, diagTol, BMCS);

		newConnection(ix+1, 1, ix+BNUM+2, .9, BHT, 7, BMCS);
		newConnection(ix+1, 2, ix+BNUM+1, .9, diag, diagTol, BMCS);

		newConnection(ix+2, 1, ix+BNUM+1, .9, BHT, 7, BMCS);
		newConnection(ix+2, 2, ix+BNUM  , .9, diag, diagTol, BMCS);

		newConnection(ix+3, 0, ix+BNUM  , .9, BHT, 7, BMCS);

/*		newNode(addNode(), x+inc*3, y-BHT/3.0, BNS, BNM, 2);
		newNode(addNode(), x+inc*3, y-BHT*2.0/3, BNS, BNM, 1);
//		newNode(addNode(), x+inc*3, y-60, BNS, BNM, 2);
		newConnection(ix+10, 0, ix+11, BCF, 20, 4, BCS);
		newConnection(ix+11, 0, ix+12, BCF, 20, 4, BCS);
//		newConnection(ix+12, 0, ix+13, BCF, 20, 4, BCS);
		newConnection(ix+10, 1, ix+3, BCF, 20, 4, BCS);*/

/*		newNode(addNode(), x+inc*2.25, y-BHT*0/9.0, BRNS, BNM, 2);
		newNode(addNode(), x+inc*2.50, y-BHT*1/9.0, BRNS, BNM, 1);
		newNode(addNode(), x+inc*2.75, y-BHT*2/9.0, BRNS, BNM, 2);
		newNode(addNode(), x+inc*2.85, y-BHT*4/9.0, BRNS, BNM, 1);
		newConnection(ix+7, 0, ix+2, BCF, 20, 4, BCS);
		newConnection(ix+7, 1, ix+8, BCF, 20, 4, BCS);
		newConnection(ix+8, 0, ix+9, BCF, 20, 4, BCS);
		newConnection(ix+9, 0, ix+10, BCF, 20, 4, BCS);
		newConnection(ix+9, 1, ix+3, BCF, 25, 4, BCS);
		newConnection(ix+10, 0, ix+11, BCF, 40, 6, BCS);
//		newConnection(ix+9, 1, ix+12, BCF, 55, 4*M_SQRT2, BCS);*/

		addBuilding(stories%2?x-80*3:x, y-BHT, stories-1);
	}
}

void lvltest(){
	initField();
	maxZoomIn = 1.5;
	newNode(addNode(), 247, 4000, 3600, 1000, 0);
	taskfixedaddLong(0, 247l, 4000l, .4);
	addBlock(0, 395, 33, 1, .7/*fric*/, 15/*spacing*/, 9/*vertSpacing*/, 6/*tol*/, 10/*str*/, 7/*size*/, 16/*mass*/);
	taskfixedadd(1, .5);
	taskfixedadd(33, .5);
	int i;
	double playerInc;
	double scoreInc;
	if(players>1){
		playerInc = 446.0/(players-1);
		scoreInc = 602.0/(players-1);
	}else playerInc=scoreInc=0;
	for(i=0; i < players; i++){
		taskguycontroladd(14+i*playerInc, 366);
		taskscoreaddLong(i, 20+i*scoreInc, 20);
	}
	//taskblorbcontrol.add(200, 363, 0);
	//addBlock(30, 336, 6, 7, .9/*fric*/, 12/*spacing*/, 7/*vertspacing*/, 6, 3, 4, 7);
	//addBlock(300, 336, 6, 7, .9/*fric*/, 12/*spacing*/, 7/*vertspacing*/, 6, 3, 4, 7);
	//taskguycontrol.addWalker(60, 360);
	taskgravityadd();
	addToolGun(247, -600);
	//taskasteroids.add(10, 8, 25);
	taskincineratoradd(410);
}

void lvltutorial(){
	initField();
	maxZoomIn = 2.0;
	players = 1;
	addBlock(0, 0, 60, 2, .95, 12, 8, 3, 5, 5, 10);
	int i = 60;
	for(; i < 121; i++) taskfixedadd(i, .4);
	taskgravityadd();
	taskincineratoradd(1550);
	tasktextadd(-40, -150, "Press 'esc' at any time to return to main menu.");
	tasktextadd(-30, -90, "Welcome!\nUse WASD to move.\nI'd try 'A' first.\nYou'll get the hang of it.");
	tasktextadd(230, -90, "Try holding 'Z' with\n'A' or 'D' to\nroll.");
	tasktextadd(500, -90, "Press 'X' to interact with\nobjects marked with squares.\nSome of these change\nthe functions of your keys.");
	newNode(addNode(), 650, 140+20*sqrt3*2, 16, 150, 4);
	int hexArg[] = {H,0,0,0,H,0,0,\
			 H,0,0,0,H,H,H,\
			  H,0,0,0,0,0,H,\
			   H,0,0,0,0,0,H,\
			    H,H,H,H,H,H,0};
	addHex(750, 140, 7, 5, hexArg, 0.90, 20, 5, 4.1, 8, 6);
	newConnection(i, 0, i+1, .8, dist(nodes+i, nodes+i+1), 15, 10);
	newConnection(i, 1, i-1, .8, dist(nodes+i, nodes+i-1), 15, 10);
	newConnection(i, 2, i+11, .8, dist(nodes+i, nodes+i+11), 15, 10);
	newConnection(i, 3, i+10, .8, dist(nodes+i, nodes+i+10), 15, 10);
	newConnection(i-1, createConnection(i-1), i+1, .8, dist(nodes+i-1, nodes+i+1), 15, 10);
	nodes[i-1].mass = 100;
	tasktextadd(730, -20, "Try falling off\nthis edge.\nHold the SAD keys\nto soften your\nlanding.");
	tasktextadd(680, 270, "If you've gotten into the stocking\nwithout breaking any limbs, congrats!\nYou can use +/- to admire your\
\nsurroundings.\n\n'esc' for main menu");
	taskguycontroladd(10*11, -30);
	addToolGun(550, -90);
}

void lvlboulder(){
	lvltest();
	int hexArg[] = {0,0,0,H,0,\
			 0,H,H,H,H,\
			  0,H,H,H,0,\
			   H,H,H,H,0,\
			    0,H,0,0,0};
	addHex(247-15*3, 330, 5, 5, hexArg, 0.80, 15, 4, 1.3, 6, 3);
}

void lvlcave(){
#define lvlcavesize 25
	initField();
	maxZoomIn = 1.5;
	if(players > 3) players = 3;
	int hexArg[] = {0,0,0,0,0,0,0,0,0,0,0,H,H,H,H,H,H,H,H,H,H,H,H,H,H,\
			 0,0,0,0,0,0,0,0,0,0,H,H,0,0,0,0,0,0,0,0,0,0,0,H,H,\
			  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
			   0,0,0,0,0,0,0,0,0,0,0,0,0,0,H,H,H,H,H,0,0,0,0,0,0,\
			    0,0,0,0,0,0,0,H,0,0,H,H,H,H,H,H,H,H,H,H,H,H,0,0,H,\
			     0,0,0,0,0,0,H,H,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,H,H,\
			      0,0,0,0,0,H,H,H,0,0,0,0,0,0,0,0,0,0,0,0,0,0,H,H,H,\
			       0,0,0,0,H,H,H,H,0,H,0,0,0,0,0,0,0,0,0,H,0,H,H,H,H,\
			        0,0,0,H,H,0,H,H,H,0,0,0,0,H,H,0,0,0,0,H,H,H,0,H,H,\
			         0,0,H,H,0,0,0,0,0,0,0,H,H,H,H,H,0,0,0,0,0,0,0,H,H,\
			          0,H,H,H,0,0,0,0,0,0,H,H,H,H,H,H,0,0,0,0,0,0,H,H,H,\
			           H,H,H,H,H,H,H,H,H,H,H,H,H,H,H,H,H,H,H,H,H,H,H,H,H};
	int ix = 114+addHex(-5*lvlcavesize, 0, 25, 12, hexArg, 0.9, lvlcavesize, 5, 3, lvlcavesize*.45, 8);
	int i = ix;
	for(; i >= ix-24; i--){
		taskfixedadd(i, 1);
	}
	taskgravityadd();
	taskincineratoradd(300);
	addScores();
	if(players < 1) return;
	taskguycontroladd(lvlcavesize*4.5, 9*sqrt3/2*lvlcavesize);
	if(players < 2) return;
	taskguycontroladd(lvlcavesize*19.5+1, 9*sqrt3/2*lvlcavesize);
	if(players < 3) return;
	taskguycontroladd(lvlcavesize*12, sqrt3/2*lvlcavesize);
}

static int addElevator(int x, int y){
	int size = 24;
	double fric = 0.90;
	int tol = 5;
	double str = 2.3;

	int ix = addNode();
	newNode(ix, x, y-200, 5, 1000, 0);
	newNode(addNode(), x-2*size, y, 13, 8, 0);
	newNode(addNode(), x+2*size, y, 13, 8, 0);
	addBlock(x+size*-5.5, y+100, 4, 1, fric, size, 0, tol, str, 11, 8);
	addBlock(x+size*1.5, y+100, 4, 1, fric, size, 0, tol, str, 11, 8);

	int i = 0;
	for(; i < 4; i++){
		connectNodes(ix+1, ix+3+i, fric, tol, str);
		connectNodes(ix+2, ix+7+i, fric, tol, str);
	}
	connectNodes(ix+1, ix+2, fric, tol, str);
	connectNodes(ix+6, ix+7, fric, tol, str);

	connectNodes(ix, ix+1, 0.7, tol*2, str*2);
	connectNodes(ix, ix+2, 0.7, tol*2, str*2);
	connectNodes(ix, ix+3, 0.7, tol*2, str*2);
	connectNodes(ix, ix+10, 0.7, tol*2, str*2);
	taskfixedadd(ix, 1);

	addBlock(x-size, y+100+sqrt3/2*size, 2, 2, fric, size, -sqrt3/2*size, tol, str*1.5, 11, 16);
//	double distance = preciseDist(ix+13, ix+1);
//	newConnection(ix+13, createConnection(ix+13), ix+1, .6, distance/2, distance, 0.015);
//	newConnection(ix+15, createConnection(ix+15), ix+2, .6, distance/2, distance, 0.015);
	//The new connection must be number 0 so that the tool works properly
	int ix2 = createConnection(ix+14);
	nodes[ix+14].connections[ix2] = nodes[ix+14].connections[0];
	double dist = preciseDist(ix, ix+14)-60;
	newConnectionLong(ix+14, 0, ix, .2, dist, dist-53, 140, .1);
	addToolToggle(ix+14);
	return ix;
}

static int addPyramid(int x, int y){
	int size = 25;
	double fric = 0.85;
	int tol = 5;
	double str = 2.3;
	int hexArg[] = {0,0,H,H,H,0,0,0,0,0,H,H,H,\
			 0,0,0,0,0,0,0,0,0,0,0,0,0,\
			  0,0,0,0,0,H,H,H,0,0,0,0,0,\
			   0,0,0,0,H,H,H,H,0,0,0,0,0,\
			    H,H,H,H,H,H,H,H,H,H,H,0,0};
	int ix = addHex(-7*size+x, y, 13, 5, hexArg, fric, size, tol, str, size*.45, 8);

	connectNodes(ix+13, ix+2, fric, tol, str);

	connectNodes(ix+23, ix+3, fric, tol, str);

	connectNodes(ix+13, ix, fric, tol, str*2);
	connectNodes(ix+14, ix, fric, tol, str);
	connectNodes(ix+16, ix, fric, tol, str*2);

	connectNodes(ix+20, ix+5, fric, tol, str*2);
	connectNodes(ix+22, ix+5, fric, tol, str);
	connectNodes(ix+23, ix+5, fric, tol, str*2);

	newNode(addNode(), x, y-200, 5, 1000, 0);
	connectNodes(ix+24, ix, 0.7, tol*2, str*2);
	connectNodes(ix+24, ix+5, 0.7, tol*2, str*2);
	connectNodes(ix+24, ix+16, 0.7, tol*2, str*2);
	connectNodes(ix+24, ix+20, 0.7, tol*2, str*2);
	taskfixedadd(ix+24, 1);
	return ix;
}

static int addSplit(int x, int y){
	int size = 25;
	double fric = 0.85;
	int tol = 5;
	double str = 2.3;
	int hexArg[] = {0,0,H,H,H,0,0,0,0,0,H,H,\
			 0,0,0,0,0,0,0,0,H,H,H,0,\
			  0,0,0,0,0,0,H,H,H,H,0,0,\
			   0,0,0,0,H,H,H,H,0,0,0,0,\
			    H,H,H,H,H,H,H,0,0,0,0,0};
	int ix = addHex(-6.5*size+x, y, 12, 5, hexArg, fric, size, tol, str, size*.45, 8);

	connectNodes(ix+16, ix, fric, tol, str*2);
	connectNodes(ix+17, ix, fric, tol, str);
	connectNodes(ix+19, ix, fric, tol, str);

//	connectNodes(ix+9, ix+6, fric, tol, str);

	newNode(addNode(), x, y-200, 5, 1000, 0);
	connectNodes(ix+23, ix, 0.7, tol*2, str*2);
	connectNodes(ix+23, ix+4, 0.7, tol*2, str*2);
	connectNodes(ix+23, ix+19, 0.7, tol*2, str*2);
	connectNodes(ix+23, ix+22, 0.7, tol*2, str*2);
	taskfixedadd(ix+23, 1);
	return ix;
}

static int addPlatform(double x, double y, int count, double spacing, double size, double mass, double fric, double tol, double str){
	int ix2 = addNode();
	newNode(ix2, x, y-200, 5, 1000, count);
	x -= (count-1)*spacing/2;
	int ix = addNode();
	newNode(ix, x, y, size, mass, 0);
	int i = 1;
	for(; i < count; i++){
		newNode(addNode(), x+spacing*i, y, size, mass, 1);
		newConnection(ix+i, 0, ix+i-1, fric, spacing, tol, str);
	}
	for(i--; i >= 0; i--){
		newConnection(ix2, i, ix+i, fric, preciseDist(ix2, ix+i), tol, str*2);
	}
	taskfixedadd(ix2, 1);
	return ix;
}

void lvlpyramid(){
	initField();
	maxZoomIn = 1.5;
	if(players > 2) players = 2;
	int ix1 = addElevator(-540, 0);
	int ix2 = addPyramid(-20, -200);
	int ix3 = addSplit(540, 0);
	int ix4 = addNode();
	newNode(ix4, -500, -225, 14, 1000, 0);
	addBridge(ix2+23, ix3, 12, .3, 14, 7, .95, 7, 4.2);
	addToolGravity(ix4+1);
	addBridge(ix1+2, ix2+13, 12, .3, 14, 7, .95, 7, 4.2);
	addBridge(ix1+10, ix3+16, 22, .2, 16, 6, .95, 7, 6);
	addBridge(ix4, ix2, 12, .3, 14, 7, .95, 7, 4.2);
	addPlatform(-710, 0, 10, 22, 10, 7, .95, 10, 4);
	taskfixedadd(ix4, 1);
	taskgravityadd();
	taskincineratoradd(300);
	if(players < 1) return;
	taskguycontroladd(-650, 25*sqrt3);
	if(players < 2) return;
	taskguycontroladd(-450, 25*sqrt3);
}

void lvlbuilding(){
	if(players>2) players = 2;
	lvltest();
	addBuilding(127, 379, 5);
}

void lvlsumo(){
	if(players > 2) players = 2;
	initField();
	maxZoomIn = 3.75;
	addBlock(180, 300, 14, 10, .83/*fric*/, 10/*spacing*/, 6.7/*vertSpacing*/, 3.3/*tol*/, 2.8/*str*/, 2/*size*/, 8/*mass*/);
	int i = 0;
	for(; i < 29; i++){
		nodes[i].size = 5;
	}
	for(i = 130; i < 145; i++){
		taskfixedadd(i, .5);
	}
	if(players > 0){
		taskguycontroladd(200, 270);
	}
	if(players > 1){
		taskguycontroladd(280, 270);
	}
	addScores();

	taskgravityadd();
	taskincineratoradd(510);
}

void lvltipsy(){
	if(players > 2) players = 2;
	initField();
	maxZoomIn = 2;
	addBlock(180, 300, 14, 20, 0.89/*fric*/, 10/*spacing*/, 6.75/*vertSpacing*/, 4.8/*tol*/, 0.6/*str*/, 4.42/*size*/, 4.0/3/*mass*/);
	//addBlock(180, 300, 14, 20, 1/*fric*/, 10/*spacing*/, 7/*vertSpacing*/, 100/*tol*/, 5/*str*/, 4/*size*/, 8/*mass*/);
	int i = 275;
	for(; i < 290; i++){
		taskfixedadd(i, .5);
	}
	if(players > 0){
		taskguycontroladdLong(220, 270, 1);
		taskscoreadd(0);
	}
	if(players > 1){
		taskguycontroladd(280, 270);
		taskscoreaddLong(1, 650, 20);
	}

	taskgravityadd();
	taskincineratoradd(710);
}

void lvltilt(){
	if(players > 2) players = 2;
	initField();
	//addBlock(185, 300, 13, 4, .7/*fric*/, 10/*spacing*/, 7/*vertSpacing*/, 10/*tol*/, 10/*str*/, 5/*size*/, 16/*mass*/);
	/*addBlock(240, 332, 2,  2, 1         , 10           ,-7               , 10        , 5         , 3        , 16       );
	for(int i = 62; i < 67; i++)
		taskfixed.add(i, (long)nodes[i].x, (long)nodes[i].y, .5);
	for(int i = 64; i < 67; i++){
		node current = nodes[i];
		node.connection[] newcon  = new node.connection[current.connections.length+2];
		newcon[current.connections.length] = current.new connection(i-9, 1, 10, 20, 3);
		System.arraycopy(current.connections, 0, newcon, 0, current.connections.length);
	}*/
	addBlock(185, 300, 13, 2, .8/*fric*/, 10/*spacing*/, 7/*vertSpacing*/, 10/*tol*/, 2.5/*str*/, 5/*size*/, 4/*mass*/);
	newNode(addNode(), 262, 370, 6, 16, 14);
	newNode(addNode(), 237, 370, 6, 16, 14);
	taskfixedadd(27, .5);
	taskfixedadd(28, .5);
	int i = 0;
	for(; i < 14; i++){
		node* target = nodes+13+i;
		newConnection(27, i, 13+i, .95, dist(nodes+27, target), 18, 0.5);
		newConnection(28, i, 13+i, .95, dist(nodes+28, target), 18, 0.5);
	}
	if(players > 0){
		taskguycontroladdLong(225, 270, 1);
		taskscoreadd(0);
	}
	if(players > 1){
		taskguycontroladd(275, 270);
		taskscoreaddLong(1, 650, 20);
	}
	taskgravityadd();
	taskincineratoradd(510);
}

static void make(){
	double size = 5;
	double mass = 9;
	double str = 6.76;
	double tol = 6.3;
newNodeLong(addNode(), 152, 308, -0.332000, 0.142000, -.574, -1.763, size, mass, 1);
newConnectionLong(8, 0, 9, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 154, 321, 0.637000, -0.785000, .072, .061, size, mass, 1);
newConnectionLong(9, 0, 10, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 157, 333, -0.274000, -0.623000, .157, .071, size, mass, 1);
newConnectionLong(10, 0, 11, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 159, 345, 0.101000, -0.654000, .042, .154, size, mass, 1);
newConnectionLong(11, 0, 12, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 162, 356, -0.169000, 0.051000, .010, .114, size, mass, 1);
newConnectionLong(12, 0, 13, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 165, 368, 0.062000, -0.416000, -.042, .238, size, mass, 1);
newConnectionLong(13, 0, 14, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 168, 379, 0.352000, -0.081000, -.098, .207, size, mass, 1);
newConnectionLong(14, 0, 15, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 172, 391, -0.265000, -0.839000, .063, -.009, size, mass, 2);
newConnectionLong(15, 0, 16, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 175, 402, -0.122000, -0.528000, .162, -.041, size, mass, 1);
newConnectionLong(16, 0, 17, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 179, 413, -0.787000, -0.378000, -.146, -.002, size, mass, 1);
newConnectionLong(17, 0, 18, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 183, 424, -0.055000, -0.814000, -.096, .023, size, mass, 1);
newConnectionLong(18, 0, 19, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 188, 434, -0.581000, -0.284000, -.089, .060, size, mass, 1);
newConnectionLong(19, 0, 20, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 192, 444, 0.295000, -0.016000, .070, -.013, size, mass, 1);
newConnectionLong(20, 0, 21, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 198, 454, -0.812000, 0.222000, .073, .148, size, mass, 1);
newConnectionLong(21, 0, 22, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 204, 463, -0.601000, 0.513000, -.069, .226, size, mass, 1);
newConnectionLong(22, 0, 23, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 210, 472, 0.334000, 0.045000, -.081, .065, size, mass, 1);
newConnectionLong(23, 0, 24, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 217, 480, 0.640000, 0.203000, .068, -.039, size, mass, 1);
newConnectionLong(24, 0, 25, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 226, 486, 0.281000, 0.705000, -.008, .294, size, mass, 1);
newConnectionLong(25, 0, 26, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 235, 491, 0.782000, 0.534000, .175, .034, size, mass, 1);
newConnectionLong(26, 0, 27, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 246, 493, 0.027000, 0.782000, .077, .071, size, mass, 1);
newConnectionLong(27, 0, 28, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 256, 494, 0.469000, -0.583000, .057, .090, size, mass, 1);
newConnectionLong(28, 0, 29, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 266, 491, 0.430000, -0.597000, -.057, -.043, size, mass, 1);
newConnectionLong(29, 0, 30, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 275, 486, 0.546000, -0.632000, .007, .191, size, mass, 1);
newConnectionLong(30, 0, 31, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 284, 479, -0.100000, -0.102000, -.129, -.060, size, mass, 1);
newConnectionLong(31, 0, 32, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 291, 471, -0.127000, -0.070000, -.013, .165, size, mass, 1);
newConnectionLong(32, 0, 33, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 298, 463, -0.408000, -0.432000, .034, .172, size, mass, 1);
newConnectionLong(33, 0, 34, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 303, 454, 0.231000, -0.749000, -.160, -.002, size, mass, 1);
newConnectionLong(34, 0, 35, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 309, 444, -0.625000, -0.387000, .047, .257, size, mass, 1);
newConnectionLong(35, 0, 36, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 313, 434, 0.558000, -0.253000, .150, .178, size, mass, 1);
newConnectionLong(36, 0, 37, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 318, 423, -0.151000, 0.384000, -.145, .028, size, mass, 1);
newConnectionLong(37, 0, 38, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 322, 413, -0.663000, -0.387000, -.111, .065, size, mass, 1);
newConnectionLong(38, 0, 39, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 325, 402, 0.496000, -0.102000, .170, .126, size, mass, 1);
newConnectionLong(39, 0, 40, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 329, 391, 0.177000, -0.046000, .119, .088, size, mass, 2);
newConnectionLong(40, 0, 41, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 332, 379, 0.235000, 0.727000, -.110, -.029, size, mass, 1);
newConnectionLong(41, 0, 42, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 335, 368, -0.289000, 0.256000, -.179, .054, size, mass, 1);
newConnectionLong(42, 0, 43, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 338, 356, -0.297000, 0.713000, .019, .144, size, mass, 1);
newConnectionLong(43, 0, 44, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 341, 345, -0.218000, 0.056000, .136, .274, size, mass, 1);
newConnectionLong(44, 0, 45, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 344, 333, -0.164000, 0.218000, .068, .282, size, mass, 1);
newConnectionLong(45, 0, 46, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 346, 321, 0.088000, 0.082000, -.155, .184, size, mass, 1);
newConnectionLong(46, 0, 47, 1.000000, 10, 10, tol, str);
newNodeLong(addNode(), 349, 308, -0.983000, 0.827000, .123, -1.702, size, mass, 0);
}
void lvlswing(){
	if(players > 2) players = 2;
	initField();
	maxZoomIn = 2.55;

	int i = 0;
	for(; i < 8; i++) newNode(addNode(), 100, 100, 2, 2, 0); //This has to be done so 'make' works properly
	
	//addBlock(150, 300, 40, 1, 1/*fric*/, 10/*spacing*/, 11/*vertSpacing*/, 10/*tol*/, 3.38/*str*/, 4.8/*size*/, 4.5/*mass*/);
	make();
	for(i = 8; i < 48; i++){
		nodes[i].y -= 100;
	}
	taskfixedaddLong(8, 150, 200, .4);
	taskfixedaddLong(47, 350, 200, .4);
	int x1 = (int)nodes[15].x;
	int x2 = (int)nodes[40].x;
	double dx = (x2-x1)/13.0;
	addBlock((int)(x1+dx/2), (int)nodes[15].y, 12, 2, .9/*fric*/, (int)dx, -dx/sqrt3, 5, 2.4/*str*/, dx*.5, 6);
	newConnection(15, 1, 48, 1, (int)dx, 5, 2.4);
	newConnection(40, 1, 59, 1, (int)dx, 5, 2.4);
	for(i = 0; i <= 11; i++){
		int target = (i <= 5)?i*2+16:i*2+17;
		newConnection(i+48, createConnection(i+48), target, .8, dist(nodes+i+48, nodes+target), 5.5, 6);
	}
	for(i = 0; i < 8; i++) {
		killNode(i);
	}
	if(players > 0){
		taskguycontroladd(200, 240);
	}
	if(players > 1){
		taskguycontroladd(280, 240);
	}
	addScores();
	taskgravityadd();
	taskincineratoradd(510);
}

void lvldrop(){
	initField();
	maxZoomIn = 3.0;
	zoom = 2;
	addBlock(30, 280, 43, 2, 1, 10, 7, 4, 6, 4.5, 10);
	//f.addBlock(
	int n, i = 0;
	for(; i < 43; i += 6){
		addToolDestroy(i);
		n = addNode();
		newNode(n, (int)nodes[i].x, 100, 10, 20, 1);
		newConnection(n, 0, i, .5, 180, 10, 3);
		taskfixedadd(n, .3);
	}
	double playerInc;
	double scoreInc;
	if(players>1){
		playerInc = 306.0/(players-1);
		scoreInc = 602.0/(players-1);
	}else playerInc=scoreInc=0;
	for(i=0; i < players; i++){
		taskguycontroladd(92+i*playerInc, 250);
		taskscoreaddLong(i, 20+i*scoreInc, 20);
	}
	taskgravityadd();
	taskincineratoradd(500);
}

#define STR_1 5
#define TOL_1 2.8
int addPlanet(int x, int y){
	int ix = addNode();
	newNode(ix, x, y, 18, 20, 6);
	addLoop(x, y, 40, 6, 0, 18, 20, 1, TOL_1, STR_1);
	addLoop(x, y, 80, 12, 0, 20, 20, 1, TOL_1, STR_1);
	int i = 0;
	for(; i < 6; i++){
		newConnection(ix, i, i+ix+1, 1, 40, TOL_1, STR_1);
		connectNodes(i+ix+1, i==0?ix+18:i*2+ix+6, 1, TOL_1, STR_1);
		connectNodes(i+ix+1, i*2+ix+7, 1, TOL_1, STR_1);
		connectNodes(i+ix+1, i*2+ix+8, 1, TOL_1, STR_1);
	}
	addLoop(x, y, 105, 66, 0, 4.8, 3, .8, 7, 2.5);
	return ix;
}
int addGenericPlanet(int x, int y, int size, double density, double surfaceDensity, double friction, double tolerance, double strength, int* layers){
	if(*layers <= 1){
		fprintf(stderr, "Strange number of layers to addGenericPlanet: %d\n", *layers);
		return -1;
	}
	int ix = addNode();
	newNode(ix, x, y, size, size*size*density, 0);
	int precedingStart = 0;
	int currentCount = 1;
	int precedingCount = 1;
	int offset = 1;
	int layer;
	int section;
	int i;
	double theta = 0;
	double s, r=size, mass, str;
	for(layer = 1; layer <= *layers; layer++){
		currentCount *= layers[layer];
		s=3*r/(currentCount-3);
		r+=s;
		s*=0.9;
		mass = s*s*(layer==*layers?surfaceDensity:density);
		str = strength*mass;
		addLoop(x, y, r, currentCount, theta, s, s*s*density, friction, tolerance, str);
		r+=s;
		theta -= M_PI/currentCount;
		if(precedingStart) connectNodes(ix+precedingStart+precedingCount-1, ix+offset, friction, tolerance, str);
		for(section = 0; section < precedingCount; section++){
			if(section) connectNodes(ix+precedingStart+section-1, ix+offset, friction, tolerance, str);
			for(i=0; i<layers[layer]; i++){
				connectNodes(ix+precedingStart+section, ix+offset+i, friction, tolerance, str);
			}
			offset+=layers[layer];
		}
		precedingStart += precedingCount;
		precedingCount = currentCount;
	}
	return ix;
}
void lvlplanet(){
	initField();
	double rads = players == 0?0:M_PI*2/players;
	register int i = 0;
	for(; i < players; i++){
		taskguycontroladd(240-130*cos(i*rads), 240+130*sin(i*rads));
	}
	taskcenteradd(addPlanet(250, 250));
	taskuniversalgravityadd(0.02);
	addToolGun(250, 250-130);
	addToolGun(250, 250+130);
	taskincinerator2add(1000);
	addScores();
}
#define rosetteSpeed 3
void lvl3rosette(){
	initField();
	maxZoomIn = 2.8;
	zoom = 2;
	double rads = players == 0?0:M_PI*2/players;
	register int i = 0;
	for(; i < players; i++){
		taskguycontroladd(240-130*cos(i*rads), 240-130*sin(i*rads));
	}
	int indexes[3];
	indexes[0] = addPlanet(250, 250);
	indexes[1] = addPlanet(-250, 250);
	indexes[2] = addPlanet(0, -183);
	double xmom = rosetteSpeed/2;
	double ymom = -sqrt3/2*rosetteSpeed;
	for(i = 0; i < players*4+85; i++){
		nodes[i].xmom = xmom;
		nodes[i].ymom = ymom;
	}
	ymom = -ymom;
	for(; i < players*4+85*2; i++){
		nodes[i].xmom = xmom;
		nodes[i].ymom = ymom;
	}
	for(; i < players*4+85*3; i++){
		nodes[i].xmom = -rosetteSpeed;
	}
	taskcenteraddLong(3, indexes);
	taskincinerator2add(1000);
	taskuniversalgravityadd(0.02);
	addScores();
}

void lvlbigplanet(){
	initField();
	zoom = 2;
	double rads = players == 0?0:M_PI*2/players;
	register int i = 0;
	for(; i < players; i++){
		taskguycontroladd(-10-300*cos(i*rads), -10+300*sin(i*rads));
	}
	int layers[] = {5, 6, 2, 1, 3, 3};
	int ix1 = addGenericPlanet(0,    0, 27, .20, .23, .97, 5, .22, layers);
	int ix2 = addNode();
	taskcenteradd(ix1);
	while(ix1 < ix2-1) addToolDestroy(++ix1);
	newNode(ix2, 0, 0, 1, 1, 0);
	killNode(ix2);//We just want the index
//	int ixs[2];
//	ixs[0]=addGenericPlanet(0,    0, 27, .20, .23, .97, 5, .22, layers);
//	ixs[1]=addGenericPlanet(0, 1000, 27, .20, .23, .97, 5, .22, layers);
//	taskcenteraddLong(2, ixs);

	taskuniversalgravityadd(0.004);
//	addToolGun(0, -300);
//	addToolGun(0, 300);
	taskincinerator2add(2000);
	addScores();
}

void lvlmech(){
	initField();
	maxZoomIn = 1.5;
	newNode(addNode(), 247, 4000, 3600, 1000, 0);
	taskfixedaddLong(0, 247l, 4000l, .4);
	addBlock(0, 395, 33, 1, .7/*fric*/, 15/*spacing*/, 9/*vertSpacing*/, 6/*tol*/, 10/*str*/, 7/*size*/, 16/*mass*/);
	taskfixedadd(1, .5);
	taskfixedadd(33, .5);
	double placeInc;
	if(players>1) placeInc = 397.0/(players-1);
	else placeInc=0;
	int i = 0;
	for(; i < players; i++){
		taskguycontroladd(49+i*placeInc, 366);
	}
	taskgravityadd();
	taskincineratoradd(410);
	addToolMech1(371, 326);
	addScores();
}

void lvlmechmech(){
	lvlmech();
	addToolMech1(124, 326);
}

void lvlmechgun(){
	lvlmech();
	addToolGun(124, 326);
}

void lvlsurvive(){
	lvltest();
	maxZoomIn = 1.3;
	taskasteroidsadd(10, 8, 25);
}
