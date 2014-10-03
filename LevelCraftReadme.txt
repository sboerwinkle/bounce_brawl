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
Pull up levels.c in your favorite text editor or IDE. Notepad++ isn't a bad choice.
Here you will see a bunch of a functions. Together they comprise a library of most of what can be done. If unsure how to reproduce something you see in a level, look at said level's code.
Let's get down to business. Go to the bottom of the file, and add these lines (minus the asterisks):
***
void lvlmylevel()
{
	lvltest();
	addBlock(-30, -115, 3, 7, 0.92, 20, 16, 5, 2, 7, 3);
}
***
The first line declares a new function, the third calls a pre-exisiting function (which sets up a good deal, not a bad subject of study), and the fourth adds a nice, medium-sized obelisk. If you look around line 200 of this file, you'll see what all those parameters mean. 'structs.h' might also be useful for explaining certain words. I've had to tweak that monolith line quite a bit; things rarely work the first time. On that note, one piece of tweaking advice: If things explode, probably your strength is too high or your friction too low (which is more aggressive, since friction of 1.0 means no friction. Look up friction in structs.h)

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



====CRAFTING A TASK====

Tasks are a bit less obvious, because you never see them. They are, however, crucially important. Everything that happens in-game and isn't a direct result of physics is driven by a task. Gravity, player control, asteroid spawning, every single tool (which are actually handled by the player control task, but we'll get to that later), scores, etc.
If you don't know what a task is made of (and you probably don't), here's a crash course before we go make one.
First of all, if you know what object orientation is, please temporarily forget that knowledge or you may get terminally confused.
Explaining this is tricky, so I guess I'll just walk through a stereotypical task's life.

--The Life of a Task--
1. Some level, while being initialized, calls 'taskderpadd', passing it the number '3'. 'taskderpadd' knows that this means the level wants node #3 to be derped every turn.
2. 'taskderpadd' creates a structure called 'taskderpdata'. This structure only holds one number, and taskderpadd sets it to 3.
3. taskderpadd creates a new 'task'. It sets its data to the taskderpdata it made, and it sets its target function to 'taskderp', which will actually do the derping. This new task gets hooked up to all the old tasks in a linked list (like a chain)
4. taskderpadd returns, and the level finished initializing itself. That task is still part of the chain. The game begins.
5. The tasks are all run in order, down the chain, eventually coming to our task.
6. Our task looks at its target function, sees that it's 'taskderp', and calls 'taskderp'. It gives taskderp a pointer to the data it has.
7. taskderp reads said data (which is a taskderpdata), and sees that it's 3. taskderp thoroughly derps node #3.
8. taskderp returns 0, meaning it isn't done yet.
9. The rest of the tasks get run.
10. Repeat steps 5-9 several times.
11. Eventually taskderp decides that node #3 is derped enough, and it returns 1. The task is removed from the chain and destroyed.
--Fin--

So, we have:
-taskderpadd, a function which creates the task
-taskderpdata, a structure which holds the data the task needs to operate.
-taskderp, a function which derps.
-task, a structure which just keeps track of which function to run and which data to give it.
Hopefully this makes sense! Now, let's make ourselves a task.

We'll begin with the main function, taskderp. Go to task.c, and, somewhere near the bottom, type:
***
static char taskderp(void *where)
{
	int i = ((taskderpdata*)where)->index;
	if (nodes[i].dead)
		return 1;
	nodes[i].size += 0.2 * SPEEDFACTOR;
	return 0;
}
***

Great. Note that we have to kill the task if the node is dead, because otherwise something might take its place and then we'd be derping the wrong thing. Also note that we use SPEEDFACTOR - Good practice dictates that, even if people affect the simulation timestep, the game should run about the same. Try to bear this in mind.

The 'data' structure, which must go above the main function, is simple:
***
typedef struct {
	int index;
} taskderpdata;
***

Now we can move onto the 'add' function. Most other add functions make good templates. This must go below taskderp, so it's defined by the time we talk about it.

First, the function declaration:
***
void taskderpadd(int ix)
{
}
***
Everything from here on out will go inside those curly braces.
Now the first thing is to make and initialize our data structure, like so:
***
taskderpdata* data = malloc(sizeof(taskderpdata));
data->index = ix;
***
If you don't know, malloc asks the computer for a chunk of memory. In this case we want a chunk which is the sizeof a taskderpdata. Malloc tells us where the chunk is, and we keep track of this location with taskderpdata* (read: taskderpdata pointer).

We can now create a task in a similar manner, and initialize its fields:
***
task *current = malloc(sizeof(task));
current->func = &taskderp;
current->data = data;
current->dataUsed = 1; // 1 means true
***
Some tasks don't need persistent data at all; in this case, we can just set dataUsed to 0 and forget about it.

Lastly, add the task to the chain.
***
addTask(current);
***

Cool. Now we need to add the "add" function to the task.h header file. This being accomplished, you can add your task from any level you chose just by calling your add function.



====CRAFTING A TOOL====

Tools are anything with a box drawn on them. They're nice, sometimes. Let's make one.

Open task.c. Look for the string "addTool". Here we are. Let's add a new function. We'll call it addToolTeleport. We need a number to serve as our tool identifier, and arbitrarily I choose 76. Some of the existing function, like addToolDestroy and addToolGrab, operate on a pre-existing node - we're going to model ours on addToolGun, which creates its node. So, below addToolGun, add the following:
***
void addToolTeleport(int x, int y)
{
	int ix = newNode(x, y, 6 /*size*/, 1 /*mass*/, 0 /*number of connections*/);
	addGenericTool(ix, 76);
}
***
Wasn't that easy? We're only just beginning >:)

The next part is actually also fairly easy. A few lines up, we should see the function getToolColor, which colors the boxes that get drawn. Let's add a case statement for our new tool, as follows. Make sure it's above 'default'.
***
case 76:
	return 0x7060FFFF;
***
This should make it a nice electric blue.

Alright, now for the real meat. Time to write the function that will make toolTeleport live up to its name. Let's call it 'toolTeleport'. We'll throw it in below toolGun, which can be found by searching for "void toolGun".
***
static void toolTeleport(taskguycontroldata *data) {
***
Note that we get a taskguycontroldata pointer, which means we get access to lots of data such as key presses, etc. We'll take advantage of this fairly quickly, by simply redirecting to taskguycontroldoLegs (which does regular leg stuff) if the action key isn't pressed.
***
if (!data->myKeys[4]) {
	data->controlVar = 0;
	taskguycontroldoLegs(data);
	return;
}
***
Otherwise, however... We've got some work to do. I'd suggest figuring out the following code for yourself, but just copying it works as well.
***
if (++data->controlVar < 10/SPEEDFACTOR)
	return;
data->controlVar = 0;
int dx = 0;
int dy = 0;
if (data->myKeys[0])
	dy -= 60;
if (data->myKeys[1])
	dx += 60;
if (data->myKeys[2])
	dy += 60;
if (data->myKeys[3])
	dx -= 60;
int i = 0;
for (; i < 4; i++) {
	if (data->exists[i]) {
		nodes[data->myNodes[i]].x += dx;
		nodes[data->myNodes[i]].y += dy;
	}
}
nodes[data->controlIndex].x += dx;
nodes[data->controlIndex].y += dy;
***
Oh, also close that curly brace from earier.
***
}
***
Almost there. Look next for the string "switch (data->controlType)". Inside this switch statement we want to add one more case:
***
case 76:
	toolTeleport(data);
	break;
***

Great! now just add 'addToolTeleport' to task.h and modify some level to use it. You should be set!
