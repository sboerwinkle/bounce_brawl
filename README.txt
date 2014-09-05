

		PLEASE! IT ISN'T LONG, I SWEAR! JUST READ THE THING!
	Or at least to the part about 'twiddling performance'. It's useful.


First off: Haven't added a license or anything yet, but:
1. You can't sue me. If something bad happens because of me, it's not my fault.
2. Feel free to distribute whatever. Just keep my name (Simon Boerwinkle) attached.

So!

====INSTALLING====

No installation, just compile and run the executable.

====COMPILING====

You'll need the following libraries: (Actually, this is just a list of the interesting stuff I linked in. If something breaks, poke around yourself or use Teh Googles to look for similar errors. Don't worry: I think it will work.)

SDL2
GLEW
GL
GLU	<- Not sure this one even matters; pretty sure it comes with any implementation of OpenGL.

----Linux----

Just run 'make'. The executable is called 'game'.

----Windows----

Ah yes. I remember the days when I had a Windows computer available to develop on. It was a nightmare.

I left the stuff in the Makefile. Assuming you have make, gcc, MinGW, etc., 'make windows' is the command to run. This may or may not work. I've added a lot since I had access to Windows.

Unfortunately, since my development computer had an Intel graphics card and couldn't use openGL to save it's life, this is a rather graphically derpy version. All the circles are octogons, though it's rather efficient. If you want to fix it, feel free to try.

----Twiddling Performance----

Don't worry, even the (moderately) computer illiterate can follow these instructions.

There are 2 "#define"s at the top of "structs.h" and 2 at the top of "gfx.c". They are well commented with descriptions of what they do. Change the values to suit your needs, and recompile.

Honestly performance shouldn't be an issue. The computer I developed this on is pretty old, and it runs the game fine with the default values.

====GAMEPLAY====

Press the buttons listed on the menu to do things. Play the tutorial. Enjoy yourself. No level progression or anything, but that's just too bad, now isn't it?

----Networking----

-Hosting

Go to manage players and add as many "NETWORKED PLAYER"s as you want. Go to the main menu and hit the host key. On the manage players screen, the slots will change from grey as people connect. Start a level to begin the lan party.

Bear in mind that every time you end a level, you will have to start hosting again.

-Connecting

Go to manage players and make sure either "PLAYER 1" or both "PLAYER 1" and "PLAYER 2" are present, depending on how many people are playing on your end. Go to the main menu and hit the connect key. If everything goes well, you should quickly see "ACKNOWLEDGED". Then wait patiently.

====FEEDBACK====

You may email questions, comments, or anything else to sboerwinkle@gmail.com. Feel free to fork on Github (http://www.github.com/sboerwinkle/bound_brawl) and do stuff or however that works.

Thing's I'm looking for especially:
 -A better name. I hate naming things.
 -A working Windows port. Won't blame you if you just don't care enough.
 -Ideas that aren't bad. Bad ideas are just the worst.
