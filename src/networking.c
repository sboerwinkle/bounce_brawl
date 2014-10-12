#include "structs.h"
#ifndef WINDOWS
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#else
#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>

	// Link with ws2_32.lib
	//#pragma comment(lib, "Ws2_32.lib") //Doesn't work
#endif
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <poll.h>
#include <GL/gl.h>
#include "gui.h"
#include "gfx.h"
#include "font.h"
#include "task.h"
#include "field.h"

int port = 4659;
char netMode = 0;
char *addressString;

static int sockfd;
static struct sockaddr_in servaddr, myaddr;
typedef struct {
	struct sockaddr_in addr;
	char dead;
	unsigned char playerNum1;
	unsigned char playerNum2;
} client;
static client *clients;

static int running;		// So keypresses can stop the connect function

static unsigned char myKeys[2];	// We could use the arrays defined in gui.h, but this way is more condusive to networking.

static uint16_t numLineBytes = 0, numCircles = 0, numPlayerCircles =
    0, numToolMarks = 0;
static uint16_t maxLineBytes = 0, maxCircles = 0, maxPlayerCircles =
    0, maxToolMarks = 0;
static uint16_t numLines = 0;
static uint8_t *lines = NULL, *circles = NULL, *playerCircles =
    NULL, *toolMarks = NULL;
static uint8_t maxClients;

static char *playerNums;

static pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t myCond = PTHREAD_COND_INITIALIZER;
static sem_t mySem, secondSem;
static volatile struct imgData *dataToSend = NULL;
static pthread_t hostThreadId;

void addNetCircle(short x, short y, unsigned int r)
{
	if (numCircles == maxCircles) {
		maxCircles += 5;
		circles = realloc(circles, maxCircles * 6);
	}
	uint16_t *data = (uint16_t *) (circles + 6 * numCircles++);
	*data = htons(x);
	*(data + 1) = htons(y);
	*(data + 2) = htons(r);
}

void addNetPlayerCircle(uint16_t ix, short x, short y, uint32_t color)
{
	if (numPlayerCircles == maxPlayerCircles) {
		maxPlayerCircles++;
		playerCircles =
		    realloc(playerCircles, 10 * maxPlayerCircles);
	}
	uint16_t *data =
	    (uint16_t *) (playerCircles + 10 * numPlayerCircles++);
	*data = htons(ix);
	*(uint32_t *) (data + 1) = htonl(color);
	data[3] = htons(x);
	data[4] = htons(y);
}

static void addNetLineSomething()
{
	if (numLineBytes == maxLineBytes) {
		maxLineBytes += 15;
		lines = realloc(lines, maxLineBytes);
	}
}

void addNetLine(uint16_t dest, uint8_t hue)
{				//This function works whether adding a new circle or a new line. Incrementing of numLines must be handled by the caller for this reason, though numLineBytes and maxLineBytes are handled in here
	addNetLineSomething();
	uint8_t *data = lines + numLineBytes;
	*((uint16_t *) data) = htons(dest);
	*(data + 2) = hue;
	numLineBytes += 3;
	return;
}

int addNetLineCircle(uint16_t ix)
{
	addNetLineSomething();
	*((uint16_t *) (lines + numLineBytes)) = htons(ix);
	numLines++;
	numLineBytes += 3;
	return numLineBytes - 1;	//This is done so the number of connected lines can be changed later.
}

void setNetLineCircleNumber(int pos, uint8_t num)
{
	lines[pos] = num;
}

void removeNetLineCircle()
{				//Undoes the indiscretions of youth
	numLineBytes -= 3;
	numLines--;
}

void addNetTool(int ix, int color)
{
	*((uint16_t *) (circles + 6 * ix + 4)) |= htons(32768);
	if (numToolMarks == maxToolMarks) {
		maxToolMarks++;
		toolMarks = realloc(toolMarks, maxToolMarks);
	}
	toolMarks[numToolMarks++] = color;
}

struct imgData {
	uint16_t sizeData;
	char *data;
	uint16_t size;
	uint16_t *centers;
};

//This function (for hosting) gets its own thread and sends out all the images to all the clients every tick.
static void sendImgs(void *derp)
{
	pthread_mutex_lock(&myMutex);
	struct imgData *myDataToSend = (struct imgData *) dataToSend;
	if (!sem_trywait(&mySem)) {	//You can't be too safe.
		if (myDataToSend != NULL) {
			free(myDataToSend->data);
			free(myDataToSend->centers);
			free(myDataToSend);
			myDataToSend = NULL;
			dataToSend = NULL;
		}
		pthread_mutex_unlock(&myMutex);
		return;
	}
	while (1) {
		while (myDataToSend == NULL) {
			pthread_cond_wait(&myCond, &myMutex);
			myDataToSend = (struct imgData *) dataToSend;
			if (!sem_trywait(&mySem)) {
				if (myDataToSend != NULL) {
					free(myDataToSend->data);
					free(myDataToSend->centers);
					free(myDataToSend);
					myDataToSend = NULL;
					dataToSend = NULL;
				}
				pthread_mutex_unlock(&myMutex);
				return;
			}
		}
		int i = 0;
		for (; i < maxClients; i++) {
			if (!clients[i].dead) {
				memcpy((uint8_t *) myDataToSend->data,
				       (uint8_t *) (myDataToSend->centers +
						    2 * i), 4);
				sendto(sockfd,
				       (char *) &myDataToSend->sizeData, 2,
				       0,
				       (struct sockaddr *) &clients[i].
				       addr, sizeof(struct sockaddr_in));
				sendto(sockfd, myDataToSend->data,
				       myDataToSend->size, 0,
				       (struct sockaddr *) &clients[i].
				       addr, sizeof(struct sockaddr_in));
			}
		}
		free(myDataToSend->data);
		free(myDataToSend->centers);
		free(myDataToSend);
		myDataToSend = NULL;
		dataToSend = NULL;
	}
}

//Another hosting function. Assembles the images and tells the above function / thread to send them out.
void writeImgs()
{
	if (!pthread_mutex_trylock(&myMutex)) {	//If trylock doesn't fail, the other thread is waiting on a condition.
		uint16_t size =
		    4 + 2 + numToolMarks + 4 + 6 * numCircles + 2 +
		    numLineBytes + 2 + 10 * numPlayerCircles;
		uint16_t sizeData = htons(size);
		uint8_t *realData = malloc(size);
		uint8_t *data = realData + 4;
		*((short *) data) = htons(numToolMarks);
		memcpy(data += 2, toolMarks, numToolMarks);
		*((short *) (data += numToolMarks)) = htons(numCircles);
		*((short *) (data += 2)) = htons(6 * maxZoomIn);
		memcpy(data += 2, circles, 6 * numCircles);
		*((short *) (data += 6 * numCircles)) = htons(numLines);
		memcpy(data += 2, lines, numLineBytes);
		*((short *) (data += numLineBytes)) =
		    htons(numPlayerCircles);
		memcpy(data += 2, playerCircles, 10 * numPlayerCircles);

		uint16_t *netCenters = malloc(2 * 2 * maxClients);
		int i = 0;
		for (; i < maxClients; i++) {
			if (clients[i].dead)
				continue;
			int x = 0, y = 0, count = 0;
			if (guyDatas[clients[i].playerNum1].alive) {
				count = 1;
				x = guyDatas[clients[i].playerNum1].
				    centerX;
				y = guyDatas[clients[i].playerNum1].
				    centerY;
			}
			if (clients[i].playerNum2 != 255
			    && guyDatas[clients[i].playerNum2].alive) {
				count++;
				x += guyDatas[clients[i].playerNum2].
				    centerX;
				y += guyDatas[clients[i].playerNum2].
				    centerY;
			}
			if (count == 2) {
				x /= 2;
				y /= 2;
			}
			netCenters[2 * i] = htons(maxZoomIn * x);
			netCenters[2 * i + 1] = htons(maxZoomIn * y);
		}

		dataToSend = malloc(sizeof(struct imgData));
		dataToSend->sizeData = sizeData;
		dataToSend->data = (char *) realData;	//At this point, it is the other thread's responsibility to free realData
		dataToSend->centers = netCenters;
		dataToSend->size = size;
		pthread_mutex_unlock(&myMutex);
		pthread_cond_signal(&myCond);
	}
	numLineBytes = 0;
	numLines = 0;
	numCircles = 0;
	numPlayerCircles = 0;
}

//Performed once, when the game starts up.
void initNetworking()
{
	addressString = malloc(16);	//xxx.xxx.xxx.xxx is 15 chars.
	strcpy(addressString, "127.0.0.1");
	sem_init(&mySem, 0, 0);
	sem_init(&secondSem, 0, 0);
#ifdef WINDOWS
	WSADATA derp;
	WSAStartup(MAKEWORD(2, 2), &derp);
#endif
}

//Performed once, when the game exits
void stopNetworking()
{
	pthread_mutex_destroy(&myMutex);
	pthread_cond_destroy(&myCond);
	sem_destroy(&mySem);
	sem_destroy(&secondSem);
	free(addressString);
#ifdef WINDOWS
	WSACleanup();
#endif
}

//Sets a player (or players!) back to gray when their client computer quits
static void decolorize(int i)
{
	requests[clients[i].playerNum1].color = 0x606060FF;
	if (clients[i].playerNum2 != 255)
		requests[clients[i].playerNum2].color = 0x606060FF;
}

//When connected, this handles our key presses.
static void keyAction(int code, char pressed)
{
	int key = 0;
	int p = 0;
	if (code == otherKeys[1]) {
		if (pressed && zoom < 32768)
			zoom *= 2;
		return;
	} else if (code == otherKeys[0]) {
		if (pressed && zoom > 1)
			zoom /= 2;
		return;
	} else if (code == SDLK_ESCAPE) {
		running = 0;
		key = 255;
	} else {
		int thing, j, i = 0;
		for (; i < 2; i++) {
			thing = 1;
			for (j = 0; j < NUMKEYS; j++) {
				if (code == pKeys[i][j]) {
					p = i;
					key = thing;
					j = NUMKEYS;
					i = 2;
				}
				thing *= 2;
			}
		}
	}
	if (key == 0)
		return;		// No need to send information, no vital keys changed.

	char old = myKeys[p];
	if (pressed)
		myKeys[p] |= key;
	else
		myKeys[p] &= 255 - key;
	if (old != myKeys[p]) {	//If it wasn't caused by a key repeat (such as "down ... downdowndowndownup"
		unsigned char vhat = myKeys[p];	// Because we're transylvanian
		if (p)
			vhat |= 128;
		sendto(sockfd, &vhat, 1, 0,
		       (struct sockaddr *) (&servaddr), sizeof(servaddr));
	}
}

static void waitForNetStuff()
{
	struct pollfd myPollFd = {.events = POLLIN,.fd = sockfd };
	SDL_Event e = {.type = SDL_USEREVENT };	//Since only the main thread can do anything of consequence, this is how we communicate that we have data to read.
	while (1) {
		if (!sem_trywait(&secondSem)) {	// If main thread sets mySem without me pushing an event, it's time to quit.
			close(sockfd);
			return;
		}
		//Wait for network input for a second, then go back to that sem_trywait real quick
		if (poll(&myPollFd, 1, 1000) == 1
		    && myPollFd.revents == POLLIN) {
			if (!SDL_PushEvent(&e))
				sem_wait(&mySem);	// If we successfully pushed the event, wait for main thread to process it.
		}
	}
}

static int netListen(int phase)
{				// Helper to myConnect. Is called whenever there's net data to read, and does all the dirty work.
	uint8_t sizeData[3];
	uint8_t *data = NULL;
	static uint16_t size = 0;
	int msgSize;
	struct sockaddr_in sender;
	socklen_t addrSize2 = sizeof(struct sockaddr_in);
	socklen_t addrSize;
	if (phase == 0) {
		uint8_t code;
		addrSize = addrSize2;
		if (0 >=
		    recvfrom(sockfd, (char *) &code, 1, 0,
			     (struct sockaddr *) &sender, &addrSize))
			return 0;
		if (code) {
			running = 0;
			return 0;
		} else {
			setColorFromHue(128);
			drawText(20 - width2, 20 + height2, TEXTSIZE,
				 "ACKNOWLEDGED");
			myDrawScreenNoClear();
			return 1;
		}
	} else if (phase == 1) {
		addrSize = addrSize2;
		msgSize =
		    recvfrom(sockfd, (char *) sizeData, 3, 0,
			     (struct sockaddr *) &sender, &addrSize);
		if (sender.sin_addr.s_addr != servaddr.sin_addr.s_addr
		    || msgSize != 2)
			return 1;
		uint8_t *tmpPointer = sizeData;
		size = ntohs(*((uint16_t *) tmpPointer));
		if (size == 0) {
			running = 0;
			return 1;
		}
		return 2;
	} else if (phase == 2) {
		glClear(GL_COLOR_BUFFER_BIT);
		data = malloc(size);
		addrSize = addrSize2;
		msgSize =
		    recvfrom(sockfd, (char *) data, size, 0,
			     (struct sockaddr *) &sender, &addrSize);
		if (sender.sin_addr.s_addr != servaddr.sin_addr.s_addr) {
			free(data);
			return 2;
		}
		if (msgSize == 2) {
			size = ntohs(*((uint16_t *) data));
			if (size == 0) {
				running = 0;
				return 1;
			}
			free(data);
			return 2;
		}
		int16_t locX = ntohs(*(int16_t *) data);
		int16_t locY = ntohs(*(int16_t *) (data + 2));
		uint8_t *pointer =
		    data + 6 + ntohs(*(uint16_t *) (data + 4));
		uint8_t *toolColors = data + 6;
		size = ntohs(*(uint16_t *) pointer);
		float myMarkSizef =
		    (float) (ntohs(*((uint16_t *) (pointer + 2))) / zoom) /
		    width2 / 2;
		pointer += 4;
		int16_t *circlePointer = (int16_t *) pointer;
		float xf, yf;
		uint16_t radius;
		int i = 0;
		char flag = 0;
		setColorWhite();
		for (; i < size; i++) {
			radius = ntohs(*((uint16_t *) (pointer + 4)));
			xf = (float) (*(int16_t *) pointer =
				      ((int16_t)
				       ntohs(*(int16_t *) pointer) -
				       locX) / zoom) / width2;
			yf = (float) (*(int16_t *) (pointer + 2) =
				      ((int16_t)
				       ntohs(*(int16_t *) (pointer + 2)) -
				       locY) / zoom) / height2;
			if (radius & 32768) {
				radius ^= 32768;
				flag = 1;
			}
			radius /= zoom;
			if (radius > 0)
				drawCircle(xf, yf,
					   (float) radius / width2);
			if (flag) {
				flag = 0;
				setColorFromHex(getToolColor
						(*(toolColors++)));
				drawRectangle(xf - myMarkSizef,
					      yf - myMarkSizef,
					      xf + myMarkSizef,
					      yf + myMarkSizef);
				setColorWhite();
			}
			pointer += 6;
		}
		size = ntohs(*(uint16_t *) pointer);
		pointer += 2;
		int j;
		int ix;
		for (i = 0; i < size; i++) {
			ix = 3 * ntohs(*(uint16_t *) pointer);
			msgSize = *(pointer += 2);
			pointer++;
			xf = (float) *(circlePointer + ix) / width2;
			yf = (float) *(circlePointer + ix + 1) / height2;
			for (j = 0; j < msgSize; j++) {
				ix = 3 * ntohs(*((uint16_t *) pointer));
				setColorFromHex(getColorFromHue
						(*(pointer += 2)));
				drawLine(xf, yf,
					 (float) *(circlePointer +
						   ix) / width2,
					 (float) *(circlePointer + ix +
						   1) / height2);
				pointer++;
			}
		}
		size = ntohs(*(uint16_t *) pointer);
		pointer += 2;
		for (i = 0; i < size; i++) {
			ix = 3 * ntohs(*(uint16_t *) pointer);
			setColorFromHex(ntohl
					(*(uint32_t *) (pointer + 2)));
			drawCircle((float) *(circlePointer + ix) / width2,
				   (float) *(circlePointer + ix +
					     1) / height2, myMarkSizef);
			drawCircle((float)
				   ((*(int16_t *) (pointer + 6) -
				     locX) / zoom) / width2,
				   (float) ((*(int16_t *) (pointer + 8) -
					     locY) / zoom) / width2,
				   myMarkSizef);
			pointer += 10;
		}
		free(data);
		myDrawScreenNoClear();
		return 1;
	}
	return 1;
}

void myConnect()
{				// Entered by pressing 'c', not exited until you push 'esc'.
	if (pIndex[0] == -1)
		return;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		return;

	memset(&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(port);

	if (0 > bind(sockfd, (struct sockaddr *) &myaddr, sizeof(myaddr))) {
		close(sockfd);
		return;
	}
	fcntl(sockfd, F_SETFL, O_NONBLOCK);

#ifndef WINDOWS
	in_addr_t addr = inet_addr(addressString);
	if (addr == (in_addr_t) (-1)) {
		close(sockfd);
		return;
	}
#else
	unsigned long addr = inet_addr(addressString);
	if (addr == -1) {
		close(sockfd);
		return;
	}
#endif

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = addr;
	servaddr.sin_port = htons(port);

	zoom = 1;

	pthread_t netStuffId;
	pthread_create(&netStuffId, NULL,
		       (void *(*)(void *)) &waitForNetStuff, NULL);

	uint32_t colors[2];
	colors[0] = htonl(requests[pIndex[0]].color);
	if (pIndex[1] == -1) {
		sendto(sockfd, (char *) colors, 4, 0,
		       (struct sockaddr *) (&servaddr), sizeof(servaddr));
	} else {
		colors[1] = htonl(requests[pIndex[1]].color);
		sendto(sockfd, (char *) colors, 8, 0,
		       (struct sockaddr *) (&servaddr), sizeof(servaddr));
	}
	myDrawScreen();
	int stage = 0;

	SDL_Event e;
	running = 1;
	myKeys[0] = 0;
	myKeys[1] = 0;
	while (running) {
		SDL_WaitEvent(&e);
		if (e.type == SDL_KEYDOWN)
			keyAction(e.key.keysym.sym, 1);
		else if (e.type == SDL_KEYUP)
			keyAction(e.key.keysym.sym, 0);
		else if (e.type == SDL_WINDOWEVENT)
			myDrawScreenNoClear();
		else if (e.type == SDL_USEREVENT) {
			stage = netListen(stage);
			sem_post(&mySem);
		} else if (e.type == SDL_QUIT) {
			SDL_PushEvent(&e);	//Push it back on so main will exit, then hand control back in that direction.
			running = 0;
		}
	}
	sem_post(&secondSem);	//Tells waitForNetStuff to kill itself
	sem_post(&mySem);	//In case he was in the middle of telling us about a packet: "Yes, yes, that't very nice, now kill yourself!"
	pthread_join(netStuffId, NULL);
	sem_trywait(&mySem);	//In case he wasn't in the middle of telling us about a packet
	glClear(GL_COLOR_BUFFER_BIT);
}

//Sets stuff up for hosting, then returns.
void myHost(int max, char *playerNumbers)
{
	playerNums = playerNumbers;

	clients = malloc(max * sizeof(client));
	maxClients = max;
	int i = 0;
	for (; i < max; i++) {
		clients[i].dead = 1;
	}
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		//Guess we didn't open the socket. :(
		free(clients);
		return;
	}

	memset(&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(port);

	if (0 > bind(sockfd, (struct sockaddr *) &myaddr, sizeof(myaddr))) {
		close(sockfd);
		free(clients);
		free(playerNums);
		return;
	}
	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	pthread_create(&hostThreadId, NULL, (void *(*)(void *)) &sendImgs,
		       NULL);

	netMode = 1;
}

void stopHosting()
{
	pthread_mutex_lock(&myMutex);
	sem_post(&mySem);
	pthread_mutex_unlock(&myMutex);
	pthread_cond_signal(&myCond);
	pthread_join(hostThreadId, NULL);
	int i = 0;
	uint16_t size = htons(0);
	for (; i < maxClients; i++) {
		if (clients[i].dead)
			continue;
		decolorize(i);
		sendto(sockfd, (char *) &size, 2, 0,
		       (struct sockaddr *) &clients[i].addr,
		       sizeof(struct sockaddr_in));
	}
	close(sockfd);
	free(clients);
	free(playerNums);
	netMode = 0;
}

//When the game begins, kick any players whose space was requested but didn't fit in the map
void kickNoRoom()
{
	int i = 0;
	uint16_t size = htons(0);
	for (; i < maxClients; i++) {
		if (clients[i].dead)
			continue;
		if (clients[i].playerNum1 < players) {
			if (clients[i].playerNum2 >= players)
				clients[i].playerNum2 = 255;
			continue;
		}
		clients[i].dead = 1;
		decolorize(i);
		sendto(sockfd, (char *) &size, 2, 0,
		       (struct sockaddr *) &clients[i].addr,
		       sizeof(struct sockaddr_in));
	}
}

void readKeys()
{
	struct pollfd myPollFd = {.fd = sockfd,.events = POLLIN };

	uint8_t *data;
	struct sockaddr_in sender;
	socklen_t size2 = sizeof(sender);
	socklen_t size;
	client *current;
	int index;

	if (mode) {
		//Handles input while the actual game is running
		data = malloc(1);
		int twoPower;
		while (poll(&myPollFd, 1, 0) == 1) {
			size = size2;
			//Poll can return false positives (I think?) so make sure we actually have something to read.
			if (0 ==
			    recvfrom(sockfd, (char *) data, 1, 0,
				     (struct sockaddr *) &sender, &size))
				continue;
			current = clients;
			for (index = 0; index < maxClients; index++) {
				//Look through, find out who it came from
				if (!current->dead
				    && current->addr.sin_addr.s_addr ==
				    sender.sin_addr.s_addr) {
					twoPower = 1;
					if (*data == 255) {
						current->dead = 1;
						fputs
						    ("Client disconnected.\n",
						     logFile);
						decolorize(index);
						break;
					}

					char *tmpKeys = (*data & 128
							 && current->
							 playerNum2 !=
							 255) ? masterKeys
					    +
					    NUMKEYS *
					    current->
					    playerNum2 : masterKeys +
					    NUMKEYS * current->playerNum1;
					for (index = 0; index < NUMKEYS;
					     index++) {
						tmpKeys[index] =
						    *data & twoPower;
						twoPower *= 2;
					}
					break;
				}
				current++;
			}
		}
	} else {
		//Handles input while in the 'lobby'
		data = malloc(8);
		int msgLen;
		client *target;
		while (poll(&myPollFd, 1, 0) == 1) {
			size = size2;
			msgLen =
			    recvfrom(sockfd, (char *) data, 8, 0,
				     (struct sockaddr *) &sender, &size);
			if (msgLen == 0)
				continue;
			current = clients;
			target = NULL;
			for (index = 0; index < maxClients; index++) {
				if (current->dead) {
					//Yes, this is an excellent place to add a player!
					if (!target) {
						target = current;
					}
				} else if (sender.sin_addr.s_addr ==
					   current->addr.sin_addr.s_addr) {
					//Hey, it's someone we already know!
					if (msgLen == 1 && *data == 255) {
						//Oh, looks like he's quitting.
						current->dead = 1;
						playerNums[current->
							   playerNum1] = 1;
						if (current->playerNum2 !=
						    255)
							playerNums
							    [current->
							     playerNum2] =
							    1;
						decolorize(index);
						fputs
						    ("Client dropped before game!\n",
						     logFile);
					}
					target = NULL;
					break;
				}
				current++;
			}
			//Code for adding a client
			if (target) {
				target->playerNum1 = target->playerNum2 =
				    255;
				for (index = 0; index < 10; index++) {
					if (playerNums[index]) {
						if (target->playerNum1 ==
						    255) {
							target->
							    playerNum1 =
							    index;
							//A message length of 4 means he only wanted to add 1 player
							if (msgLen == 4)
								break;
						} else {
							target->
							    playerNum2 =
							    index;
							break;
						}
					}
				}
				if (index == 10)
					*data = 1;
				else {
					target->addr = sender;
					target->dead = 0;
					playerNums[target->playerNum1] = 0;
					requests[target->playerNum1].
					    color =
					    ntohl(*(uint32_t *) data);
					if (msgLen != 4) {
						playerNums[target->
							   playerNum2] = 0;
						requests[target->
							 playerNum2].
						    color =
						    ntohl(*
							  (((uint32_t *)
							    data) + 1));
					}
					fputs("Stored a client.\n",
					      logFile);
					*data = 0;
				}
			} else
				*data = 1;
			sendto(sockfd, (char *) data, 1, 0,
			       (struct sockaddr *) &sender, size);
		}
	}
	free(data);
}
