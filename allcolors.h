typedef struct OctTree octtree;
typedef struct SuperColor supercolor;
typedef struct ColorPointer colorpointer;

void putColorInTree(octtree* tree, colorpointer color);
void removeFromTree(octtree* tree, colorpointer color);
void putColorInChildTree(octtree* tree, colorpointer color);
int hasPointInside(octtree* tree, colorpointer color);
void splitOctTree(octtree* tree);
colorpointer findNearestColorInTree(octtree* tree, colorpointer nom, colorpointer nearest);
int shouldVisitTree(octtree* tree, colorpointer nom, colorpointer nearest);

int getColorDistance(colorpointer color, colorpointer other);
