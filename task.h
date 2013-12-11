extern int* taskguycontrolindexes;

extern void runTask(task **where);

extern void freeAllTasks(task* where);

extern void taskaicombatadd(int Player);

extern void taskasteroidsadd(int s, int m, int c);

extern void taskcenteradd(int i);

extern void taskcenteraddLong(int len, int* i);

extern void taskdestroyadd(int i, int t);

extern void taskfixedaddLong(int i, long x, long y, double s);

extern void taskfixedadd(int i, double s);

extern void taskfrictionadd();

extern void taskgravityadd();

extern void taskguycontroladdLong(int x, int y, Sint8 flipped);

extern void taskguycontroladd(int x, int y);

extern void taskguycontroladdToolMech1(int x, int y);

extern void taskguycontroladdGenericTool(int ix, int type);

extern void taskguycontroladdToolDestroy(int ix);

extern void taskguycontroladdToolGrab(int ix);

extern void taskguycontroladdToolToggle(int ix);

extern void taskincineratoradd(long height);

extern void taskincinerator2add(long height);

extern void taskpointgravityadd(int ix, double gravity);

extern void taskuniversalgravityadd(double gravity);

extern void taskscoreaddLong(int ix, int x, int y);

extern void taskscoreadd(int ix);
