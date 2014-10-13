
This work is released into the public domain.
I would say, however, that if you try to monetize anything that you base off of this or not include the source code, I hope a faceless terror haunts you for the remainder of your pitiful days.
I would also prefer that you keep these few lines included. My name is Simon Boerwinkle.
fontData.h is the work of A. Schiffler - aschiffler@ferzkopp.net. See fontData.h for license information.

So!

Contents:
=Compiling
 -Linux
 -Windows
 -Twiddling Performance
=Gameplay
 -Networking
 -Achievements
=Levelcraft
=Feedback/Contributing

====COMPILING====

You'll need the following libraries: (Actually, this is just a list of the interesting stuff I linked in. If something breaks, poke around yourself or use Teh Googles to look for similar errors. Don't worry: I think it will work.)

SDL2
GLEW
GL
GLU	<- Not sure this one even matters; pretty sure it comes with any implementation of OpenGL.

----Linux----

cd into src, run 'make'. The executable is called 'game', and is put in the top level directory.
#cd src
#make
#cd ..
#./game

If you're a total newbie, I'll try to give you a hand here. Google anything you don't understand.
First, open a terminal.
Use 'cd' to change directory to where this is.
Type in the commands above, without the '#'s
If it says you don't have make, pull up your package manager. You'll need make, gcc, and all the packages above (dev versions, if that's an option)
Eventually it should work.

----Windows----

You should have a pre-compiled binary sitting around, game.exe. Just run that. If you want to compile from source for whatever reason, bravo. Here's what you have to do...

Get MinGW. A couple of links, the first you'll definitely need and the second you might need if you're not sure what to install:
http://www.mingw.org/wiki/Getting_Started
http://www.mingw.org/wiki/howto_install_the_mingw_gcc_compiler_suite

Great. Now go to the SDL2 site, and get the development version for mingw. You may need 7-Zip or something similar to extract this file. Once this is done, you should see in the extracted folder a pair of folders ending in "mingw32". Pick one (I'm trying i686 first) and copy all of its contents into the corresponding places in C:\MinGW.

Whew. Now for GLEW. You're gonna wanna build these from source, which is by no means easy, but necessary. Grab the source zip from the glew site. Extract, and go into the folder. Hooray, a makefile! But it doesn't work for windows. Instead, make your own .bat file and type in the following: (from sourceforge)
***
gcc -DGLEW_NO_GLU -O2 -Wall -W -Iinclude  -DGLEW_BUILD -o src/glew.o -c src/glew.c
gcc -shared -Wl,-soname,libglew32.dll -Wl,--out-implib,lib/libglew32.dll.a    -o lib/glew32.dll src/glew.o -L/mingw/lib -lglu32 -lopengl32 -lgdi32 -luser32 -lkernel32
ar cr lib/libglew32.a src/glew.o

gcc -DGLEW_NO_GLU -DGLEW_MX -O2 -Wall -W -Iinclude  -DGLEW_BUILD -o src/glew.mx.o -c src/glew.c
gcc -shared -Wl,-soname,libglew32mx.dll -Wl,--out-implib,lib/libglew32mx.dll.a -o lib/glew32mx.dll src/glew.mx.o -L/mingw/lib -lglu32 -lopengl32 -lgdi32 -luser32 -lkernel32
ar cr lib/libglew32mx.a src/glew.mx.o
***
Don't put in the lines of '*'s, those are just markers. Run the batch file. Look in the 'lib' folder, and copy the contents to \MinGC\lib. If you build a different version of glew than me (1.11.0), it may be necessary to replace the 'glew32.dll' I provide (in this folder) with the one you just made when it comes time to run your game.

What you're *supposed* to be able to do now is open a command prompt, go into this folder, go into 'src', and type 'mingw32-make windows'.

This doesn't work for me, which is unsurprising when you've been developing stuff for windows and linux. My first error is something about 'winapifamily'. Best I can figure, SDL2 thinks I'm on windows 8 (I'm not). Pull up \MinGw\include\SDL2\SDL_platform.h, and around line 121 you should see the offending 'include'. We'll just comment out that whoooole section, so one line up, where it says
#if [blah blah blah]
change it to
#if 0 && [blah blah blah]
That fixed it for me. I next had a whole slew of other errors, but I fixed those permanently. They shouldn't be a problem for you, but heaven knows what will be. Google any errors, email me if you have no other options.

If, upon running the game and starting a level, you notice that you can't see anything, try going to the top of 'src/gfx.c', uncommenting the 'STUPIDINTEL' line, and recompiling. It makes all your circles into octogons :)

One more footnote: 'mingw32-make cleanWindows' is also valid. This deletes all the .o files and the executable, if you want to force a rebuild. Shouldn't need it.

----Twiddling Performance----

Don't worry, even the (moderately) computer illiterate can follow these instructions.

There are 2 "#define"s at the top of "structs.h" and 2 at the top of "gfx.c". They are well commented with descriptions of what they do. Change the values to suit your needs, and recompile.

Honestly performance shouldn't be an issue. The computer I developed this on is pretty old, and it runs the game fine with the default values.

====GAMEPLAY====

Press the buttons listed on the menu to do things. Play the tutorial. Enjoy yourself. Fighting AIs or other people or going for all the achievements are sort of the intended purposes, but do what you want.

The default controls are set up to minimize keyboard ghosting (look it up), so change at your own risk (especially w/ two people)

----Networking----

-Hosting

Go to manage players and add as many "NETWORKED PLAYER"s as you want. Go to the main menu and hit the host key. On the manage players screen, the slots will change from grey as people connect. Start a level to begin the lan party.

Bear in mind that every time you end a level, you will have to start hosting again.

-Connecting

Go to manage players and make sure either "PLAYER 1" or both "PLAYER 1" and "PLAYER 2" are present, depending on how many people are playing on your end. Go to the main menu and hit the connect key. If everything goes well, you should quickly see "ACKNOWLEDGED". Then wait patiently.

----Achievements----

Every level has an associated achievement. If you press 'v' on the menu, you can see what they're called - this should give you a hint as to what to do. A few things to note:

The achievements are checked when you quit the level and return to the menu. The achievements don't care about what you did earlier, they only care about the state of the game on the instant that you quit. This means you'll never have to do something 5 times, etc. since that doesn't show up when you look at a snapshot of the game.

Completing the achievement earns you a pair of stars by the level's name. Completing it with no one taking any damage or respawning earns you the stars, plus that level appears in technicolor. If you earn the entire technicolor base menu, consider the game beaten. Congratulations, sir or madam - not many have the patience and skill for what you have accomplished.

The achievements vary widely in difficulty. If you can't figure out how to beat it, you either haven't wasted enough lives or haven't tried something interesting. If you know how to beat it but always get hurt in the process, try a different achievement. I guarantee that they're all beatable (I've beaten them), but some are
Just
So
Hard.

Any and all cheats are acceptable for earning the achievements. Didn't know there were cheats, did you? That's your reward for reading this thing. All of them can be activated from the menu, where an indicator will appear. Additionally, some can be toggled in-game. You don't need the cheats to win (they aren't *that* great), but they can be helpful. F6 is the most fun, if not immediately obvious how it works. Credit to Adam Bliss for naming that one. All cheats are a single keystroke, which should make things pretty easy... except I plan for one more that involves the Konami code at the right time. We'll see if I get around to it.

If you want to move the achievements.dat file so you can re-earn some of them, feel free. Not like I care. If you want to edit the achievements.dat file to give yourself a feeling of self-worth, bad karma. Bad Karma.

====LEVELCRAFT====

LevelCraftReadme.txt contains fairly detailed walkthroughs on level, task, and tool creation. This can be a rewarding experience for those who have the time and programming skill. It does require programming, but even the novice should be able to, if nothing else, make some pretty hilarious changes to the masses of the particles :) Know that it does require the ability to compile the game from source, which might be tricky on Windows. You've been warned.
I should point out that I haven't tried this out on any actual people - While I did test all the code I describe, it's possible that I missed something or that I didn't explain an important point. This makes the next section all the more important.

====FEEDBACK/CONTRIBUTING====

You may email questions, comments, bug reports, crash reports, history reports, fan mail, death threats, chain letters, death mail, chain threats, fan letters, chainmail, fan threats, death letters, death notes, fans, death, chains, cute pictures of cats, interesting news articles, broken links, strangely ominous strings of characters, additions to this list, or anything else to sboerwinkle@gmail.com. If and when I cast this out to the internet, hearing even a "plunk" in reply would be rewarding. The world is frighteningly full of people.

If you wish to contribute to the project... Gosh, I'm flattered! I know Github has a way to fork projects, but I've never looked into it. Unless I'm mistaken, you fork it and then I authorize a merge when you're done. Rest assured, I'll do my best to honor the effort anyone is willing to put into this thing, even if I have to create a mod system! Shoot me an email, I guess.

Thing's I'm looking for especially:
 -A better name. I hate naming things.
 -Levels.
 -Ideas that aren't bad. Bad ideas are just the worst.
