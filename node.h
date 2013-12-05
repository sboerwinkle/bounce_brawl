
void newConnectionLong(int node, int con, int id, double fric, double prefLen, double mid, double tol, double force);

void newConnection(int node, int con, int id, double fric, double len, double tol, double force);

void newNodeLong(int where, long X, long Y, double Px, double Py, double Xm, double Ym, double S, double M, int array);

void newNode(int where, int x, int y, double size, double mass, int array);

int createConnection(int who);

void positioncleanup(node* who);
