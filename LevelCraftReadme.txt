Welcome!

This readme is intended to help with 3 related tasks, namely:
+Creating levels
+Creating tasks
+Creating tools

Some things you will need, however: The program 'make', and all the other tools for compiling. If you're on a linux machine right now, you either know how to work 'make' or you know someone who does. You had to compile the game somehow. Feel free to skip this next paragraph.
If you're on a Windows machine... the news is worse. It's probably possible to compile this with Visual Studio, but there ain't no way (pardon my grammar) that I'm going to write a guide for that. You're probably running the version I compiled for you, for which I used minGW and make and maybe some other stuff. If you're not very computer savvy or adventurous, you'd honestly better turn back now. I'm sorry, but there's a reason I didn't create this game on Windows. It's much less easy (for me, at least). I mean, minGW is a minor nightmare to get working the first time, and don't you have to pay for a Visual Studio license? That's a load of crap. Paying money to develop software for fun. If you know a nice, free, easy way to compile on Windows, tell me. Otherwise, look into linux. Linux newbies might like Ubuntu. Regardless of which distro (= distribution = version) you chose, there's a good chance you'll get a package manager. Using that, it's easy as falling to install 'make', 'gcc', 'sdl2-dev', and anything else you might need. You'll probably have to google a few errors, as I don't know the full list of packages I used. But once you're set up, you just type 'make' into the console and it friggin' works. For free. I didn't mention earlier that Linux and many things for it are free. Free. Or hell, mac w/ Xcode. But that's not free.

I should probably mention at this point that a bit of knowledge on C is helpful. If you're willing to learn, that's enough; it's not like we're doing any multi-threaded sorting algorithms. Careful study of what's already there combined with the internet should probably suffice.

So, here we go!



====CRAFTING A LEVEL====

Hooray! Enthusiasm! Basically, here are the main steps we'll go through:
1. Write the code that describes the level
2. Add the level to the header file, so all the other files know it exists
3. Add the level to the menu

STEP 1
Pull up levels.c in your favorite text editor.
Here you will see a bunch of a functions. Together they comprise a library of most of what can be done. If unsure how to reproduce something you see in a level, look at said level's code.
Let's get down to business. Go to the bottom of the file, and add these lines (minus the asterisks):
***
void lvlmylevel()
{
	lvltest();
	addBlock(-30, -115, 3, 7, 0.92, 20, 16, 5, 2, 7, 3);
}
***
The first line declares a new function, the third calls a pre-exisiting function (which sets up a good deal, not a bad subject of study), and the fourth adds a nice, medium-sized obelisk. If you look around line 200 of this file, you'll see what all those parameters mean. 'node.h' might also be useful for explaining certain words. I've had to tweak that monolith line quite a bit; things rarely work the first time. On that note, one piece of tweaking advice: If things explode, probably your strength is too high or your friction too low (which is more aggressive, since friction of 1.0 means no friction. Look up friction in node.h)

STEP 2
Much simpler. Open levels.h (the header file), go to the bottom, and add an entry for your new level. The syntax should be obvious. This lets all the other files know that your function exists (For all you inexperienced C programmers).

STEP 3
Open gui.c. Here things get a bit hairy. Jump down to around line 812. You should see a bunch of lines, bearing the names of all your favorite levels! Time to add yours.
First thing: make space in one of the menus. The top menu is a bit peculiar, so we'll avoid it for now. Instead, look for the line starting with:
***
menuItem *flatMenu =
***
This is where the flat menu is created. Change that '4' to a '5'. Hooray progress!
Go down to the group of 4 lines, all of which start with
***
addMenuLevel(flatMenu, 
***
Add a 5th! It should read:
***
addMenuLevel(flatMenu, lvlmylevel, achieveLazy, "MY FIRST LEVEL", "JUST PLAY IT!");
***
This adds to 'flatMenu' the level 'lvlmylevel' with the name 'MY FIRST LEVEL', the achievement 'achieveLazy' (which awards the achievement to anyone who plays it), and the achievement text 'JUST PLAY IT!'. If you wish to craft an achievement, that isn't really covered here, but suffice it to say that it's much the same as steps 1 and 2 above, but with achievements.c

That's it! Compile and run, and the level should be there in the "FLAT STAGES..." menu. I should mention that this will muck up the achievements, because there's a different number of levels now than when the achievements.dat file was saved. You have my permission to cheat yourself back to your original achievements, if you can figure out how :) Also, note that adding an odd number of players spawns one guy in the obelisk. This is also a problem with the boulder level. Just a note.
