#CC=i686-pc-mingw32-gcc
CC=gcc
CFLAGS=-Wall -O2 -c -pthread $(DEBUG)

.PHONY: debug windows remake clean

game: field.o font.o gui.o levels.o node.o task.o gfx.o networking.o
	$(CC) $(DEBUG) field.o font.o gui.o levels.o node.o task.o gfx.o networking.o -o game $(LFLAGS) -lSDL2 -lm -pthread

debug:
	$(MAKE) DEBUG="-g -O0"
	
windows:
	$(MAKE) DEBUG="-D WINDOWS" LFLAGS="-ILib/SDL2/x86_64-w64-mingw32/include/SDL2 -Dmain=SDL_main -LLib/SDL2/x86_64-w64-mingw32/lib -lmingw32 -lSDL2main -mwindows -lws2_32 -o game.exe"

remake:
	$(MAKE) clean
	$(MAKE)

field.o: field.c structs.h node.h gui.h task.h gfx.h
	$(CC) $(CFLAGS) field.c

font.o: font.c
	$(CC) $(CFLAGS) font.c

gui.o: gui.c structs.h font.h levels.h field.h networking.h
	$(CC) $(CFLAGS) gui.c

levels.o: levels.c structs.h field.h task.h node.h gui.h
	$(CC) $(CFLAGS) levels.c

node.o: node.c structs.h field.h
	$(CC) $(CFLAGS) node.c

task.o: task.c structs.h gui.h field.h node.h font.h gfx.h networking.h
	$(CC) $(CFLAGS) task.c

gfx.o: gfx.c gfx.h
	$(CC) $(CFLAGS) gfx.c

networking.o: networking.c structs.h font.h gfx.h gui.h field.h
	$(CC) $(CFLAGS) networking.c

clean:
	rm -f *.o game game.exe

cleanWindows:
	del *.o game game.exe
