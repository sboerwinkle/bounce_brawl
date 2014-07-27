extern task* firstTask;

extern void addTask(task* who);

extern void runTask(task **where);

extern void freeAllTasks(task* where);

extern void taskaicombatadd(int Player);

extern void taskaispacecombatadd(int Player);

extern void taskasteroidsadd(int s, int m, int c);

extern void taskcenteradd(int i);

extern void taskcenteraddLong(int len, int* i);

extern void taskdestroyadd(int i, int t);

extern void taskfixedaddLong(int i, long x, long y, double s);

extern void taskfixedadd(int i, double s);

extern void taskinflateadd(int i, double step, double max);

extern void taskfrictionadd();

extern void taskgravityadd();

extern void taskguycontroladd(int x, int y);

extern uint32_t getToolColor(int type);

extern void addGenericTool(int ix, int type);

extern void addToolMech1(int x, int y);

extern void addToolGun(double x, double y);

extern void addToolDestroy(int ix);

extern void addToolGrab(int ix);

extern void addToolToggle(int ix);

extern void addToolGravity(int ix);

extern void taskincineratoradd(long height);

extern void taskincinerator2add(long height);

extern void taskpointgravityadd(int ix, double gravity);

extern void taskuniversalgravityadd(double gravity);

extern void taskscoreaddLong(int ix, int x, int y);

extern void taskscoreadd(int ix);

extern void tasktextadd(int x, int y, char* text);
