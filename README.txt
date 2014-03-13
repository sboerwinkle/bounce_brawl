

		PLEASE! IT ISN'T LONG, I SWEAR! JUST READ THE THING!
	Or at least to the part about 'twiddling performance'. It's useful.


First off: Haven't added a license or anything yet, but:
1. You can't sue me. If something bad happens because of me, it's not my fault.
2. Feel free to distribute whatever. Just keep my name (Simon Boerwinkle) attached.

So!

====INSTALLING====

You'll need the following libraries: (Actually, this is just a list of the interesting stuff I linked in. If something breaks, poke around yourself or use Teh Googles to look for similar errors.)

SDL2
GLEW
GL
GLU	<- Not sure this one even matters; pretty sure it comes with any implementation of OpenGL.

----Linux----

Just run 'make'. The executable is called 'game'.

----Windows----

Doesn't work yet. I got it working with MinGW and the Windows version of 'make', but after the switch to OpenGL I realized I just didn't care enough to weed out the bugs. So have fun, if it really matters to you.

----Twiddling Performance----

Don't worry, even the (moderately) computer illiterate can follow these instructions.

There are 2 "#define"s at the top of "structs.h", and 1 at the top of "gfx.c". They are well commented with descriptions of what they do. Change the values to suit your needs, and recompile.

Honestly performance shouldn't be an issue; the computer I developed this on is pretty old.

====GAMEPLAY====

Press the buttons listed on the menu to do things. Play the tutorial. Enjoy yourself. No level progression or anything, but that's just too bad, now isn't it?

====FEEDBACK====

You may email questions, comments, or anything else to sboerwinkle@gmail.com. Feel free to fork on Github (http://www.github.com/sboerwinkle/bound_brawl) and do stuff or however that works.

Thing's I'm looking for especially:
 -A better name. I hate naming things.
 -A working Windows port. Won't blame you if you just don't care enough.
 -Ideas that aren't bad. Bad ideas are just the worst.
