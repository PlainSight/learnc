typedef struct OctTree octtree;
typedef struct SuperColor supercolor;

void putColorInTree(octtree* tree, supercolor* color);
void removeFromTree(octtree* tree, supercolor* color);
void putColorInChildTree(octtree* tree, supercolor* color);
int hasPointInside(octtree* tree, supercolor* color);
void splitOctTree(octtree* tree);

int getColorDistance(supercolor* color, supercolor* other);