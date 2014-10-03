
extern void newConnectionLong(int node, int con, int id, double fric,
			      double prefLen, double mid, double tol,
			      double force);

extern void newConnection(int node, int con, int id, double fric,
			  double len, double tol, double force);

extern int newNodeLong(long X, long Y, double Px, double Py, double Xm,
		       double Ym, double S, double M, int array);

extern int newNode(int x, int y, double size, double mass, int array);

extern int createConnection(int who);

extern void positioncleanup(node * who);
