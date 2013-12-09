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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "gui.h"
#include "gfx.h"
#include "font.h"
#include "field.h"

int port = 4659;
Sint8 netMode = 0;
char* addressString;

static int sockfd;
static struct sockaddr_in servaddr, myaddr;
typedef struct{
	struct sockaddr_in addr;
	char dead;
	Sint8 playerNum;
} client;
static client *clients;

static int running; // So keypresses can stop the connect function
static Sint8 netListenKill = 0;

static char myKeys; // We could use the arrays defined in gui.h, but this way is more condusive to networking.

static Uint16 numLineBytes = 0, numCircles = 0, numPlayerCircles = 0;
static Uint16 maxLineBytes = 0, maxCircles = 0, maxPlayerCircles = 0;
static Uint16 numLines = 0;
static Uint8 *lines = NULL, *circles = NULL, *playerCircles = NULL;
static Uint8 numClients, maxClients;

static pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t myCond = PTHREAD_COND_INITIALIZER;
static struct imgData* dataToSend = NULL;

void addNetCircle(short x, short y, unsigned int r){
	if(numCircles == maxCircles){
		maxCircles += 5;
		circles = realloc(circles, maxCircles*6);
	}
	Uint16* data = (Uint16*)(circles+6*numCircles++);
	*data = htons(x);
	*(data+1) = htons(y);
	*(data+2) = htons(r);
}
void addNetPlayerCircle(Uint16 ix, Uint32 color){
	if(numPlayerCircles == maxPlayerCircles){
		maxPlayerCircles++;
		playerCircles = realloc(playerCircles, 6*maxPlayerCircles);
	}
	Uint16* data = (Uint16*)(playerCircles + 6*numPlayerCircles++);
	*data = htons(ix);
	*(Uint32*)(data+1) = htonl(color);
}
static void addNetLineSomething(){
	if(numLineBytes == maxLineBytes){
		maxLineBytes += 15;
		lines = realloc(lines, maxLineBytes);
	}
}
void addNetLine(Uint16 dest, Uint8 hue){//This function works whether adding a new circle or a new line. Incrementing of numLines must be handled by the caller for this reason, though numLineBytes and maxLineBytes are handled in here
	addNetLineSomething();
	Uint8* data = lines + numLineBytes;
	*((Uint16*)data) = htons(dest);
	*(data+2) = hue;
	numLineBytes += 3;
	return;
}
int addNetLineCircle(Uint16 ix){
	addNetLineSomething();
	*((Uint16*)(lines+numLineBytes)) = htons(ix);
	numLines++;
	numLineBytes += 3;
	return numLineBytes-1;//This is done so the number of connected lines can be changed later.
}
void setNetLineCircleNumber(int pos, Uint8 num){
	lines[pos] = num;
}
void removeNetLineCircle(){//Undoes the indiscretions of youth
	numLineBytes -= 3;
	numLines--;
}
void addNetTool(int ix){
	*((Uint16*)(circles+6*ix+4)) |= htons(32768);
}
/*void addLine(Uint8 hue, short x1, short y1, short x2, short y2){
	if(numLines == maxLines){
		maxLines += 5;
		lines = realloc(lines, maxLines*9);
	}
	Uint8* data = lines+9*numLines++;
	*data = hue;
	*((short*)(data+1)) = htons(x1);
	*((short*)(data+3)) = htons(y1);
	*((short*)(data+5)) = htons(x2);
	*((short*)(data+7)) = htons(y2);
}*/

struct imgData{
	Uint16 sizeData;
	char* data;
	Uint16 size;
	Uint16* centers;
};

static void sendImgs(void* derp){
	pthread_mutex_lock(&myMutex);
	if(netListenKill){//You can't be too safe.
		if(dataToSend!=NULL){
			free(dataToSend->data);
			free(dataToSend->centers);
			free(dataToSend);
			dataToSend = NULL;
		}
		pthread_mutex_unlock(&myMutex);
		netListenKill = 0;
		return;
	}
	while(1){
		while(dataToSend==NULL){
			pthread_cond_wait(&myCond, &myMutex);
			if(netListenKill){
				if(dataToSend!=NULL){
					free(dataToSend->data);
					free(dataToSend->centers);
					free(dataToSend);
					dataToSend = NULL;
				}
				pthread_mutex_unlock(&myMutex);
				netListenKill = 0;
				return;
			}
		}
		int i = 0;
		for(; i < numClients; i++){
			if(!clients[i].dead){
				memcpy((Uint8*)dataToSend->data, (Uint8*)(dataToSend->centers+2*i), 4);
				sendto(sockfd, (char*)&dataToSend->sizeData, 2, 0, (struct sockaddr*)&clients[i].addr, sizeof(struct sockaddr_in));
				sendto(sockfd, dataToSend->data, dataToSend->size, 0, (struct sockaddr*)&clients[i].addr, sizeof(struct sockaddr_in));
			}
		}
		free(dataToSend->data);
		free(dataToSend->centers);
		free(dataToSend);
		dataToSend = NULL;
	}
}

void writeImgs(){
	if(numClients && !pthread_mutex_trylock(&myMutex)){//If trylock doesn't fail, the other thread is waiting on a condition.
		Uint16 size = 4+4+6*numCircles+2+numLineBytes+2+6*numPlayerCircles;
		Uint16 sizeData = htons(size);
		Uint8* realData = malloc(size);
		Uint8* data = realData+4;
		*((short*)data) = htons(numCircles);
		*((short*)(data+=2)) = htons(6*maxZoomIn);
		memcpy(data+=2, circles, 6*numCircles);
		*((short*)(data+=6*numCircles)) = htons(numLines);
		memcpy(data+=2, lines, numLineBytes);
		*((short*)(data+=numLineBytes)) = htons(numPlayerCircles);
		memcpy(data+=2, playerCircles, 6*numPlayerCircles);

		Uint16* netCenters = malloc(2*2*numClients);
		int i = 0;
		for(; i < numClients; i++){
			if(clients[i].dead) continue;
			netCenters[2*i] = htons(maxZoomIn * centers[clients[i].playerNum].x);
			netCenters[2*i+1] = htons(maxZoomIn * centers[clients[i].playerNum].y);
		}

		dataToSend = malloc(sizeof(struct imgData));
		dataToSend->sizeData = sizeData;
		dataToSend->data = (char*)realData;//At this point, it is the other thread's responsibility to free realData
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

void initNetworking(){
	addressString = malloc(16); //xxx.xxx.xxx.xxx is 15 chars.
	strcpy(addressString, "127.0.0.1");
#ifdef WINDOWS
	WSADATA derp;
	WSAStartup(MAKEWORD(2, 2), &derp);
#endif
}

void stopNetworking(){
	pthread_mutex_destroy(&myMutex);
	pthread_cond_destroy(&myCond);
	free(addressString);
#ifdef WINDOWS
	WSACleanup();
#endif
}

static void keyAction(int code, char pressed){
	if(code == SDL_SCANCODE_MINUS){
		if(pressed && zoom < 32768) zoom *= 2;
		return;
	}else if(code == SDL_SCANCODE_EQUALS){
		if(pressed && zoom > 1) zoom /= 2;
		return;
	}else if(code == SDL_SCANCODE_ESCAPE){
		running = 0;
		code = 255;
	}else if(code == SDL_SCANCODE_UP) code = 1;
	else if(code == SDL_SCANCODE_RIGHT) code = 2;
	else if(code == SDL_SCANCODE_DOWN) code = 4;
	else if(code == SDL_SCANCODE_LEFT) code = 8;
	else if(code == SDL_SCANCODE_RCTRL) code = 16;
	else if(code == SDL_SCANCODE_RSHIFT) code = 32;
	else return; // No need to send information, no vital keys changed.

	char old = myKeys;
	if(pressed)	myKeys |= code;// Hence the resetting of 'code' shown above.
	else 		myKeys &= 255-code;
	if(old!=myKeys)//If it wasn't caused by a key repeat (such as "down ... downdowndowndownup"
		sendto(sockfd, &myKeys, 1, 0, (struct sockaddr*)(&servaddr), sizeof(servaddr));
}

static void netListen(void* color){ // Helper to myConnect. Listens for frames and draws them. Kills itself when netListenKill is set
	Uint32 Color = htonl(*(Uint32*)color);
	sendto(sockfd, (char*)&Color, 4, 0, (struct sockaddr*)(&servaddr), sizeof(servaddr));
	struct timeval wait, waitClone = {.tv_sec = 1, .tv_usec = 0};
	fd_set* fdSet = malloc(sizeof(fd_set));
	FD_ZERO(fdSet);
	Uint8* sizeData = malloc(3);
	Uint8*  data = NULL;
	Uint16 size = 0;
	int msgSize;
	struct sockaddr_in sender;
	socklen_t addrSize = sizeof(struct sockaddr_in);
	SDL_Event e = {.type = SDL_WINDOWEVENT}; //Since only the main thread can manipulate the window, thie event is our way of signalling a screen update.
	memset(screen, 0, 750*750*4);
	SDL_PushEvent(&e); 
	do{
		if(netListenKill){
			free(fdSet);
			free(sizeData);
			close(sockfd);
			netListenKill = 0;
			return;
		}
		FD_SET(sockfd, fdSet);
		wait = waitClone;
	}while(!select(sockfd+1, fdSet, NULL, NULL, &wait));
	{
		Uint8 code;
		recvfrom(sockfd, (char*)&code, 1, 0, (struct sockaddr*)&sender, &addrSize);
		if(code) running = 0;
		else drawText(screen, 20, 20, 0x00FF00FF, TEXTSIZE, "ACKNOWLEDGED");
	}
	SDL_PushEvent(&e);
	while(1){
		if(size == 0){
			do{
				if(netListenKill){
					free(fdSet);
					free(sizeData);
					close(sockfd);
					netListenKill = 0;
					return;
				}
				FD_SET(sockfd, fdSet);
				wait = waitClone;
			}while(!select(sockfd+1, fdSet, NULL, NULL, &wait));
			msgSize = recvfrom(sockfd, (char*)sizeData, 3, 0, (struct sockaddr*)&sender, &addrSize);
			if(sender.sin_addr.s_addr != servaddr.sin_addr.s_addr){
				puts("E: Info from someone besides the server!");
				continue;
			}
			if(msgSize != 2){
				puts("E: Read frame as length data");
				continue;//That was a frame, but we only got the first 2 bytes...
			}
			size = ntohs(*((Uint16*)sizeData));
			if(size == 0){//Don't return, because we still need to flip netListenKill back to 0 and perform cleanup whenever main thread figures out what's going on and tries to kill us.
				running = 0;
				SDL_PushEvent(&e);//Gives it an event so it notices running is now 0.
			}
			data = malloc(size);
		}
		do{
			if(netListenKill){
				free(data);
				free(sizeData);
				free(fdSet);
				close(sockfd);
				netListenKill = 0;
				return;
			}
			FD_SET(sockfd, fdSet);
			wait = waitClone;
		}while(!select(sockfd+1, fdSet, NULL, NULL, &wait));
		FD_ZERO(fdSet);
		msgSize = recvfrom(sockfd, (char*)data, size, 0, (struct sockaddr*)&sender, &addrSize);
		while(sender.sin_addr.s_addr != servaddr.sin_addr.s_addr){
			puts("E: Info from someone besides the server! ");
			msgSize = recvfrom(sockfd, (char*)data, size, 0, (struct sockaddr*)&sender, &addrSize);
		}
		if(msgSize == 2){
			puts("E: Read length data as frame");
			size = ntohs(*((Uint16*)data));
			if(size == 0){
				running = 0;
				SDL_PushEvent(&e);//Gives it an event so it notices running is now 0.
			}
			free(data);
			data = malloc(size);
			continue;
		}
		memset(screen, 0, 750*750*4);
		Uint16 locX = ntohs(*(Uint16*)data);
		Uint16 locY = ntohs(*(Uint16*)(data+2));
		size = ntohs(*(Uint16*)(data+4));
		Uint16 myMarkSize = ntohs(*((Uint16*)(data+6)))/zoom;
		if(myMarkSize < 2) myMarkSize = 2;
		Uint8* pointer = data + 8;
		Uint16* circlePointer = (Uint16*)pointer;
		short x, y;
		Uint16 radius;
		int i = 0;
		for(; i < size; i++){
			radius = ntohs(*((Uint16*)(pointer+4)));
			x = ( *(Uint16*)pointer = (ntohs(*(Uint16*)pointer)-locX)/zoom+375 );
			y = ( *(Uint16*)(pointer+2) = (ntohs(*(Uint16*)(pointer+2))-locY)/zoom+375 );
			if(radius & 32768){
				radius ^= 32768;
				rectangleColor(screen, x-myMarkSize/2, y-myMarkSize/2, myMarkSize, myMarkSize, 0xFFFFFFFF);
			}
			radius /= zoom;
			if(radius > 0)
				ellipseColor(screen, x, y, radius, radius, 0xFFFFFFFF);
			pointer += 6;
		}
		size = ntohs(*(Uint16*)pointer);
		pointer += 2;
		int j;
		int ix;
		for(i = 0; i < size; i++){
			ix = 3*ntohs(*(Uint16*)pointer);
			msgSize = *(pointer+=2);
			pointer++;
			x = *(circlePointer+ix);
			y = *(circlePointer+ix+1);
			for(j = 0; j < msgSize; j++){
				ix = 3*ntohs(*((Uint16*)pointer));
				lineColor(screen, x, y, *(circlePointer+ix), *(circlePointer+ix+1), getColorFromHue(*(pointer+=2)));
				pointer++;
			}
		}
		size = ntohs(*(Uint16*)pointer);
		pointer+=2;
		for(i = 0; i < size; i++){
			ix = 3*ntohs(*(Uint16*)pointer);
			ellipseColor(screen, *(circlePointer+ix), *(circlePointer+ix+1), myMarkSize/2, myMarkSize/2, ntohl(*(Uint32*)(pointer+2)));
			pointer+=6;
		}
		size = 0;//since we use 'size' to determine which variety of packet to read next, this says we just read a screenshot.
		SDL_PushEvent(&e); // Here: I recommend a flag that sets whether or not the last frame finished drawing.
		free(data);
	}
}

void myConnect(Uint32 color){ // Entered by pressing 'c', not exited until you push 'esc'.
	if(netListenKill) return; // The last one hasn't finished exitting.
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) return;
	
	memset(&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	myaddr.sin_port=htons(port);
	
	if(0 > bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr))){
		close(sockfd);
		return;
	}

#ifndef WINDOWS
	in_addr_t addr = inet_addr(addressString);
	if(addr == (in_addr_t)(-1)){
		close(sockfd);
		return;
	}
#else
	unsigned long addr = inet_addr(addressString);
	if(addr == -1){
		close(sockfd);
		return;
	}
#endif

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=addr;
	servaddr.sin_port=htons(8080);//Here: convert all the '8080's to 'port's : 8080 is just for testing porpoises on the same computer.

	zoom = 1;

	{ // I don't care about the ID, I trust it to exit. But I have to store the value. So I litmit its scope.
		pthread_t netListenID;
		pthread_create(&netListenID, NULL, (void* (*)(void*))&netListen, &color);
		pthread_detach(netListenID);
	}

	SDL_Event e;
	running = 1;
	myKeys = 0;
	while(running){
		SDL_WaitEvent(&e);
		if(e.type == SDL_KEYDOWN)	keyAction(e.key.keysym.scancode, 1);
		else if(e.type == SDL_KEYUP)	keyAction(e.key.keysym.scancode, 0);
		else if(e.type == SDL_WINDOWEVENT) myDrawScreen();
		else if(e.type == SDL_QUIT){
			SDL_PushEvent(&e);//Push it back on so main will exit, then hand control back in that direction.
			running = 0;
		}
	}
	netListenKill = 1; // socket is closed whenever netListen gets the message here sent.
}
void myHost(int max, int* playerNumbers){
	if(netListenKill) return;

	clients = malloc(max*sizeof(client));
	maxClients = max;
	numClients = 0;
	int i = 0;
	for(; i < max; i++){
		clients[i].dead = 1;
		clients[i].playerNum = playerNumbers[i];
	}
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0){
		free(clients);
		return;
	}
	
	memset(&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	myaddr.sin_port=htons(8080);
	
	if(0 > bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr))){
		close(sockfd);
		free(clients);
		return;
	}

	pthread_t id;
	pthread_create(&id, NULL, (void* (*)(void*))&sendImgs, NULL);
	pthread_detach(id);

	netMode = 1;
}

void stopHosting(){
	pthread_mutex_lock(&myMutex);
	netListenKill = 1;
	pthread_cond_signal(&myCond);
	pthread_mutex_unlock(&myMutex);
	int i = 0;
	Uint16 size = htons(0);
	for(; i < numClients; i++){
		if(clients[i].dead) continue;
		requests[clients[i].playerNum].color = 0x606060FF;
		sendto(sockfd, (char*)&size, 2, 0, (struct sockaddr*)&clients[i].addr, sizeof(struct sockaddr_in));
	}
	close(sockfd);
	free(clients);
	netMode = 0;
}

void kickNoRoom(){ // When the game begins, kick any players whose space was requested but didn't fit in the map
	int i = numClients-1;
	Uint16 size = htons(0);
	for(; i >= 0; i--){
		if(clients[i].playerNum < players) return; // 'playerNum's are in descending order, so if one has is still in the game, all the rest will be, too.
		if(clients[i].dead) continue;
		clients[i].dead = 1;
		requests[clients[i].playerNum].color = 0x606060FF;
		sendto(sockfd, (char*)&size, 2, 0, (struct sockaddr*)&clients[i].addr, sizeof(struct sockaddr_in));
	}
}

void readKeys(){
	struct timeval* zeroTime = calloc(1, sizeof(struct timeval));
	fd_set* fdSet = malloc(sizeof(fd_set));
	FD_ZERO(fdSet);
	FD_SET(sockfd, fdSet);

	Uint8* data;
	struct sockaddr_in sender;
	socklen_t size = sizeof(sender);
	client* current;
	int index;

	if(mode){
		data = malloc(1);
		int twoPower;
		while(select(sockfd+1, fdSet, NULL, NULL, zeroTime)){
			recvfrom(sockfd,(char*) data, 1, 0, (struct sockaddr*)&sender, &size);
			current = clients;
			for(index = 0; index < numClients; index++){
				if(!current->dead && current->addr.sin_addr.s_addr == sender.sin_addr.s_addr){
					twoPower = 1;
					for(index = 0; index < NUMKEYS; index++){
						masterKeys[NUMKEYS*current->playerNum + index] = *data & twoPower;
						twoPower *= 2;
					}
					if(*data == 255){
						current->dead = 1;
						puts("Client disconnected.");
						requests[current->playerNum].color = 0x606060FF;
					}
					break;
				}
				current++;
			}
		}
	}else{
		data = malloc(5);
		int msgLen;
		client* target;
		while(select(sockfd+1, fdSet, NULL, NULL, zeroTime)){
			msgLen = recvfrom(sockfd, (char*)data, 5, 0, (struct sockaddr*)&sender, &size);
			current = clients;
			target = NULL;
			for(index = 0; index < maxClients; index++){
				if(current->dead){
					if(!target){
						target = current;
						puts("Found a place to store him");
					}
				}else if(sender.sin_addr.s_addr == current->addr.sin_addr.s_addr){
					if(msgLen == 1){
						if(*data == 255){
							current->dead = 1;
							requests[current->playerNum].color = 0x606060FF;
							puts("Client dropped before game!");
						}
						target = NULL;
					}else{
						puts("Client requested reassign...");
						target = current;
						numClients--;//If it's a reconnect, counteract the ++ below.
					}
					break;
				}
				current++;
			}
			if(target){
				target->addr = sender;
				target->dead = 0;
				requests[target->playerNum].color = ntohl(*(Uint32*)data);
				puts("Stored him.");
				numClients++;
				*data = 0;
			}else *data = 1;
			sendto(sockfd, (char*)data, 1, 0, (struct sockaddr*)&sender, size);
		}
	}
	free(zeroTime);
	free(fdSet);
	free(data);
}
