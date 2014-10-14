#include <stdlib.h>
#include <stdio.h>
#include "structs.h"
#include "field.h"
#include "gui.h"
#include "math.h"

//For when playing the level is enough
int achieveLazy()
{
	return 1;
}

//Automatically called for every level. Determines if you get the rainbow-level achievement.
int achieveFlawless()
{
	int i = 0;
	for (; i < players; i++) {
		if (guyDatas[i].injured || !guyDatas[i].firstLife)
			return 1;
	}
	return 2;
}

//Most achievements are named after their level
int achieveGardens()
{
	int i = 0;
	for (; i < players; i++) {
		if (guyDatas[i].controlType == 70)
			return 1;
	}
	return 0;
}

int achieveCave()
{
	int i = 0;
	for (; i < players; i++) {
		if (abs(guyDatas[i].centerX) < 50
		    && guyDatas[i].centerY < -50)
			return 1;
	}
	return 0;
}

int achieveBuilding()
{
	int i = 0;
	for (; i < players; i++) {
		if (guyDatas[i].centerY < -325)
			return 1;
	}
	return 0;
}

//For levels which require only a certain number of nodes to remain
static int achieveDestruction(numAcceptable)
{
	int alive = -4 * players - numAcceptable;
	int i = 0;
	for (; i < numNodes; i++) {
		if (!nodes[i].dead)
			if (0 == alive++)
				return 0;
	}
	return 1;
}

int achieveSumo()
{
	return achieveDestruction(70);
}

int achieveDrop()
{
	return achieveDestruction(20);
}

int achieveWalled()
{
	return achieveDestruction(20);
}

int achieveBoulder()
{
	int i = 0;
	for (; i < numNodes; i++) {
		if (!nodes[i].dead && 3 == nodes[i].mass)
			return 0;
	}
	return 1;
}

int achievePlain()
{
	int i = 0;
	for (; i < numNodes; i++) {
		if (!nodes[i].dead && 1 == nodes[i].mass)
			return 0;
	}
	return 1;
}

int achieveManMech()
{
	int count = 0;
	int result = 0;
	int i = 0;
	int j;
	for (; i < numNodes; i++) {
		if (nodes[i].dead || 18 != nodes[i].mass)
			continue;
		count++;
		for (j = 0; j < nodes[i].numConnections; j++) {
			if (nodes[i].connections[j].dead) {
				result = 1;
				break;
			}
		}
	}
	if (count < 5)
		return 0;
	return result;
}

int achieveGunMech()
{
	int i = 0;
	for (; i < numNodes; i++) {
		if (!nodes[i].dead
		    && (18 == nodes[i].mass || 1 == nodes[i].mass))
			return 0;
	}
	return 1;
}

int achieveMechMech()
{
	unsigned char stage = 0;
	int pos[2] = { -1000, 1000 };;
	int i = 0;
	int j, k, l;
	for (; i < numNodes; i++) {
		if (nodes[i].dead || nodes[i].mass != 18
		    || nodes[i].size != 6)
			continue;
		for (j = nodes[i].numConnections - 1; j >= 0; j--) {
			if (nodes[i].connections[j].dead)
				return 0;
			k = nodes[i].connections[j].id;
			if (nodes[k].dead)
				return 0;
			for (l = nodes[k].numConnections - 1; l >= 0; l--)
				if (nodes[k].connections[l].dead)
					return 0;
			if (stage ^ (nodes[k].x > pos[stage]))
				pos[stage] = nodes[k].x;
		}
		if (stage)
			return pos[0] < pos[1];
		else
			stage++;
	}
	return 0;
}

int achieveAsteroids()
{
	int i = 0, j;
	for (; i < numNodes; i++) {
		if (nodes[i].dead || nodes[i].size != 10)
			continue;
		for (j = 0; j < players; j++) {
			double dx = guyDatas[j].centerX - nodes[i].x;
			double dy = guyDatas[j].centerY - nodes[i].y;
			if (dx * dx + dy * dy <= 100)
				return 1;
		}
	}
	return 0;
}

int achievePlanet()
{
	int i = 0;
	for (; i < players; i++) {
		double x = guyDatas[i].centerX;
		double y = guyDatas[i].centerY;
		if (sqrt(x * x + y * y) > 390)
			return 1;
	}
	return 0;
}

static void achieveRosetteHelper(int ix, char *touching)
{
	touching[ix] = 1;
	int x = nodes[ix].x;
	int y = nodes[ix].y;
	int r = nodes[ix].size;
	int i = 0;
	for (; i < numNodes; i++) {
		if (nodes[i].dead || touching[i])
			continue;
		double dx = nodes[i].x - x;
		double dy = nodes[i].y - y;
		if (sqrt(dx * dx + dy * dy) - nodes[i].size - r < 10)
			achieveRosetteHelper(i, touching);
	}
}

int achieveRosette()
{
	int i;
	for (i = 0; i < numNodes && (nodes[i].dead || nodes[i].size == 6);
	     i++);
	if (i == numNodes) {
		puts("WTF HOW\nPlz contact me!\n-Simon");	// They killed everything. how.
		return 1;
	}
	char *touching = calloc(numNodes, 1);
	achieveRosetteHelper(i, touching);
	for (i = 0; i < numNodes; i++)
		if (nodes[i].dead == 0 && touching[i] == 0
		    && nodes[i].size != 6) {
			free(touching);
			return 0;
		}
	free(touching);
	return 1;
}

int achieveBigPlanet()
{
	int i = 0;
	for (; i < numTools; i++) {
		if (tools[i].type == 0 && tools[i].where != -1)
			return 0;
	}
	return 1;
}

int achieveTutorial()
{
	int count = 0;
	int i = 0;
	for (; i < numNodes; i++) {
		if (nodes[i].dead == 0 && nodes[i].size == 8) {
			if (++count == 9)
				return 0;
		}
	}
	return 1;
}
