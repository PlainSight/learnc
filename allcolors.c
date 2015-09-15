#include<stdio.h>
#include<stdlib.h>
#include "allcolors.h"
#include "lodepng.h"

#define ARRAYSIZE 16

struct SuperColor {
	int x;
	int y;
	int r;
	int g;
	int b;
	octtree* location;
};

struct OctTree {
	int minx;
	int miny;
	int minz;
	int maxx;
	int maxy;
	int maxz;

	supercolor* colors[ARRAYSIZE];
	int size;

	octtree* parent;
	octtree* children[8];

	int hasChildren;
};

supercolor* createSuperColor(int r, int g, int b) {

	supercolor* color = (supercolor *) malloc(sizeof(supercolor));

	color->r = r;
	color->g = g;
	color->b = b;

	printf("made a color\n");

	return color;
}

octtree* createOctTree(int minx, int miny, int minz,
					   int maxx, int maxy, int maxz, 
					   octtree *p) {
	octtree* tree = (octtree *) malloc(sizeof(octtree));

	tree->parent = p;

	tree->minx = minx;
	tree->maxx = maxx;

	tree->miny = miny;
	tree->maxy = maxy;

	tree->minz = minz;
	tree->maxz = maxz;

	printf("made a tree\n");

	return tree;
}

int hasPointInside(octtree* tree, supercolor* color) {
	return tree->minx <= color->r && color->r < tree->maxx 
		&& tree->miny <= color->g && color->g < tree->maxy 
		&& tree->minz <= color->b && color->b < tree->maxz;
}

int getColorDistance(supercolor* color, supercolor* other) {
	int rd = color->r - other->r;
	int gd = color->g - other->g;
	int bd = color->b - other->b;

	return rd*rd + gd*gd + bd*bd;
}

void putColorInChildTree(octtree* tree, supercolor* color) {
	for(int i = 0; i < 8; i++) {
		if(hasPointInside(tree->children[i], color)) {
			putColorInTree(tree->children[i], color);
		}
	}
}

void removeFromTree(octtree* tree, supercolor* color) {
	for(int i = 0; i < ARRAYSIZE; i++) {
		if(tree->colors[i] == color) {
			tree->colors[i] = tree->colors[tree->size - 1];
			break;
		}
	}

	for(octtree* t = tree; t != NULL; t = t->parent) {
		t->size--;

		if(t->size < ARRAYSIZE/2 && t->hasChildren) {

			int tColorIndex = 0;

			//foreach child get all the colors
			for(int i = 0; i < 8; i++) {
				for(int j = 0; j < t->children[i]->size; j++) {
					t->colors[tColorIndex++] = t->children[i]->colors[j];
				}
				t->children[i]->hasChildren = 0;
				t->children[i]->size = 0;
			}

			for(int i = 0; i < t->size; i++) {
				t->colors[i]->location = t;
			}

		}
	}
}

void putColorInTree(octtree* tree, supercolor* color) {
	if(tree->hasChildren) {
		putColorInChildTree(tree, color);
	} else {
		if(tree->size < ARRAYSIZE) {
			tree->colors[tree->size] = color;
			color->location = tree;
		} else {
			splitOctTree(tree);
			putColorInChildTree(tree, color);
		}
	}
}

int shouldVisitTree(octtree* tree, supercolor* nom, supercolor* nearest) {
	int aa = nom->r - (tree->minx + tree->maxx) / 2;
	int bb = nom->g - (tree->miny + tree->maxy) / 2;
	int cc = nom->b - (tree->minz + tree->maxz) / 2;

	float dd = 0.71f * (tree->maxx - tree->minx);

	float distancesqr = (aa*aa + bb*bb + cc*cc) - dd*dd;

	return getColorDistance(nom, nearest) > distancesqr;
}

supercolor* findNearestColorInTree(octtree* tree, supercolor* nom, supercolor* nearest) {
	if(tree->size == 0 || (nearest == NULL && !shouldVisitTree(tree, nom, nearest))) {
		return nearest;
	}

	if(!tree->hasChildren) {
		for(int i = 0; i < tree->size; i++) {
			if(nearest == NULL) {
				nearest = tree->colors[i];
			} else {
				if(getColorDistance(nom, tree->colors[i]) < getColorDistance(nom, nearest)) {
					nearest = tree->colors[i];
				}
			}
		}
		return nearest;
	} else {
		for(int i = 0; i < 8; i++) {
			supercolor* temp = findNearestColorInTree(tree->children[i], nom, nearest);

			if(temp == NULL) continue;
			
			if(nearest == NULL) {
				nearest = temp;
			} else {
				if(getColorDistance(nom, temp) < getColorDistance(nom, nearest)) {
					nearest = temp;
				}
			}
		}
		return nearest;
	}
}

void splitOctTree(octtree* tree) {
	int midx = ( tree->minx + tree->maxx ) / 2;
	int midy = ( tree->miny + tree->maxy ) / 2;
	int midz = ( tree->minz + tree->maxz ) / 2;

	if(!tree->children[0]) {
		tree->children[0] = createOctTree(tree->minx, tree->miny, tree->minz,
										  		midx, 		midy, 		midz, 
										  tree);
		tree->children[1] = createOctTree(tree->minx, tree->miny, 		midz,
										  		midx, 		midy, tree->maxz, 
										  tree);
		tree->children[2] = createOctTree(tree->minx, 		midy, tree->minz,
										  		midx, tree->maxy, 		midz, 
										  tree);
		tree->children[3] = createOctTree(tree->minx, 		midy, 		midz,
										  		midx, tree->maxy, tree->maxz, 
										  tree);

		tree->children[4] = createOctTree(		midx, tree->miny, tree->minz,
										  tree->maxx, 		midy, 		midz, 
										  tree);
		tree->children[5] = createOctTree(		midx, tree->miny, 		midz,
										  tree->maxx, 		midy, tree->maxz, 
										  tree);
		tree->children[6] = createOctTree(		midx, 		midy, tree->minz,
										  tree->maxx, tree->maxy, 		midz, 
										  tree);
		tree->children[7] = createOctTree(		midx, 		midy, 		midz,
										  tree->maxx, tree->maxy, tree->maxz, 
										  tree);
	}

	tree->hasChildren = 1;

	for (int i = 0; i < ARRAYSIZE; i++) {
		putColorInChildTree(tree, tree->colors[i]);
	}
}

void outImage(const char* filename, const unsigned char* image, unsigned width, unsigned height)
{
  /*Encode the image*/
  unsigned error = lodepng_encode32_file(filename, image, width, height);

  /*if there's an error, display it*/
  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
}

int main(int argc, char *argv[]) {

	octtree* root = createOctTree(0, 0, 0, 255, 255, 255, NULL);

	printf("Hello World\n");

	unsigned int width = 512;
	unsigned int height = 512;

	printf("alloc mem\n");

	unsigned char* image = (unsigned char*) malloc(width * height * 4);

	printf("alloc mem succ\n");

	outImage("picture.png", image, width, height);

	free(image);
	
}
