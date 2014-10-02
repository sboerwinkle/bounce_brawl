

		PLEASE! IT ISN'T LONG, I SWEAR! JUST READ THE THING!
		          Or at least the next three lines


First off: Haven't added a license or anything yet, but:
1. You can't sue me. If something bad happens because of me, it's not my fault.
2. Feel free to distribute whatever. Just keep my name (Simon Boerwinkle) attached.

So!

Contents:
=Installing
=Compiling
 -Linux
 -Windows
 -Twiddling Performance
=Gameplay
 -Networking
 -Achievements
=Levelcraft
=Feedback/Contributing

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

If you're a total newbie, I'll try to give you a hand here. Google anything you don't understand.
First, open a terminal.
Use 'cd' to change directory to where this is.
type in 'make', press enter.
If it says you don't have make, pull up your package manager. You'll need make, gcc, and all the packages above (dev versions, if that's an option)
When make succeeds (doesn't say 'error'), type in './game'. Enjoy.

----Windows----

Ah yes. I remember the days when I had a Windows computer available to develop on. It was a nightmare.

I left the stuff in the Makefile. Assuming you have make, gcc, MinGW, etc., 'make windows' is the command to run. This may or may not work. I've added a lot since I had access to Windows.

Unfortunately, since my development computer had an Intel graphics card and couldn't use openGL to save it's life, this is a rather graphically derpy version. All the circles are octogons, though this makes it exra-speedy. If you want to fix it, feel free to try.

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

I plan to have a separate readme for all the ins and outs of creating new levels, as this can be a rewarding experience for those who have the time and programming skill. It does require programming, but even the novice should be able to, if nothing else, make some pretty hilarious changes to the masses of the particles :)

====FEEDBACK/CONTRIBUTING====

You may email questions, comments, fan mail, cute pictures of cats, intersting news articles, broken links, strangely ominous strings of characters, or anything else to sboerwinkle@gmail.com. If and when I cast this out to the internet, hearing even a "plunk" in reply would be rewarding.

If you wish to contribute to the project... Gosh, I'm flattered! I know Github has a way to fork projects, but I've never looked into it. Unless I'm mistaken, you fork it and then I authorize a merge when you're done. Rest assured, I'll do my best to honor the effort anyone is willing to put into this thing, even if I have to create a mod system! Shoot me an email, I guess.

Thing's I'm looking for especially:
 -A better name. I hate naming things.
 -A working Windows port. Won't blame you if you just don't care enough.
 -Ideas that aren't bad. Bad ideas are just the worst.

That last one was, of course, sarcasm. There are no stupid ideas, only stupid people.
