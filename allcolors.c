#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#include "allcolors.h"
#include "lodepng.h"

#define ARRAYSIZE 32

struct SuperColor {
	octtree* location;
	int x;
	int y;
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

struct OctTree {
	supercolor* colors[ARRAYSIZE];
	octtree* children[8];
	octtree* parent;
	int minx;
	int miny;
	int minz;
	int maxx;
	int maxy;
	int maxz;
	int size;
	//hasChildren, 0 = no and not initialized, 1 = no but initialized, 2 = yes
	unsigned short hasChildren;
};

supercolor createSuperColor(int r, int g, int b) {

	supercolor color;
	color.r = (unsigned char) r;
	color.g = (unsigned char) g;
	color.b = (unsigned char) b;

	return color;
}

octtree* createOctTree(int minx, int miny, int minz,
	int maxx, int maxy, int maxz,
	octtree *p) {
	octtree* tree = (octtree *)malloc(sizeof(octtree));

	tree->parent = p;

	tree->minx = minx;
	tree->maxx = maxx;

	tree->miny = miny;
	tree->maxy = maxy;

	tree->minz = minz;
	tree->maxz = maxz;

	tree->hasChildren = 0;
	tree->size = 0;

	return tree;
}

int hasPointInside(octtree* tree, supercolor* color) {

	return tree->minx <= color->r && color->r < tree->maxx 
		&& tree->miny <= color->g && color->g < tree->maxy 
		&& tree->minz <= color->b && color->b < tree->maxz;
}

int getColorDistance(supercolor* color, supercolor* other) {

	int rd = (int) color->r - (int) other->r;
	int gd = (int) color->g - (int) other->g;
	int bd = (int) color->b - (int) other->b;

	return rd*rd + gd*gd + bd*bd;
}

void putColorInChildTree(octtree* tree, supercolor* color) {

	for(int i = 0; i < 8; i++) {
		if(hasPointInside(tree->children[i], color)) {
			putColorInTree(tree->children[i], color);
			return;
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
		(t->size)--;

		if(t->size < ARRAYSIZE/2 && t->hasChildren == 2) {

			int tColorIndex = 0;

			//foreach child get all the colors
			for(int i = 0; i < 8; i++) {
				for (int j = 0; j < t->children[i]->size; j++) {
					t->colors[tColorIndex++] = t->children[i]->colors[j];
				}
				t->children[i]->hasChildren = t->children[i]->hasChildren == 2 ? 1 : 0;
				t->children[i]->size = 0;
			}

			for(int i = 0; i < t->size; i++) {
				t->colors[i]->location = t;
			}

			t->hasChildren = 1;
		}
	}
}

void putColorInTree(octtree* tree, supercolor* color) {

	if(tree->hasChildren == 2) {
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

	(tree->size)++;
}

int shouldVisitTree(octtree* tree, supercolor* nom, supercolor* nearest) {

	//mid values

	int mx = (tree->minx + tree->maxx) / 2;
	int my = (tree->miny + tree->maxy) / 2;
	int mz = (tree->minz + tree->maxz) / 2;

	//closest corner values

	int cx = (nom->r < mx) * tree->minx + (nom->r > mx) * tree->maxx;
	int cy = (nom->g < my) * tree->miny + (nom->g > my) * tree->maxy;
	int cz = (nom->b < mz) * tree->minz + (nom->b > mz) * tree->maxz;

	int isoutx = (nom->r >= tree->maxx) | (nom->r < tree->minx);
	int isouty = (nom->g >= tree->maxy) | (nom->g < tree->miny);
	int isoutz = (nom->b >= tree->maxz) | (nom->b < tree->minz);

	int dx = isoutx * (((int)nom->r) - cx);
	int dy = isouty * (((int)nom->g) - cy);
	int dz = isoutz * (((int)nom->b) - cz);

	//this distance is accurate if the color value is not within any min max range
	int distancesqr = dx*dx + dy*dy + dz*dz;

	return getColorDistance(nom, nearest) > distancesqr;
}

supercolor* findNearestColorInTree(octtree* tree, supercolor* nom, supercolor* nearest) {

	if(tree->size == 0 || (nearest != NULL && !shouldVisitTree(tree, nom, nearest))) {
		return nearest;
	}

	if(tree->hasChildren < 2) {
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
		//get a good first nearest candidate

		int midx = (tree->maxx + tree->minx) / 2;
		int midy = (tree->maxy + tree->miny) / 2;
		int midz = (tree->maxz + tree->minz) / 2;

		int bestChild = 4 * (nom->r > midx)
									+	2 * (nom->g > midy)
									+			(nom->b > midz);

		nearest = findNearestColorInTree(tree->children[bestChild], nom, nearest);

		for(int i = 0; i < 8; i++) {
			if (i == bestChild) continue;

			nearest = findNearestColorInTree(tree->children[i], nom, nearest);
		}
		return nearest;
	}
}

void splitOctTree(octtree* tree) {

	int midx = ( tree->minx + tree->maxx ) / 2;
	int midy = ( tree->miny + tree->maxy ) / 2;
	int midz = ( tree->minz + tree->maxz ) / 2;

	if(tree->hasChildren == 0) {
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

	tree->hasChildren = 2;

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

void setPixel(unsigned char* image, int width, int height, supercolor* color, octtree* tree, int* pseudoRandom) {
	int set = 0;

	int openSpaces[8][2];

	while(!set) {
		
		supercolor* closestNeighbour = findNearestColorInTree(tree, color, NULL);
						
		int minx = closestNeighbour->x - 1;
		int maxx = closestNeighbour->x + 1;
		int miny = closestNeighbour->y - 1;
		int maxy = closestNeighbour->y + 1;

		minx = minx < 0 ? 0 : minx;
		maxx = maxx >= width ? width-1 : maxx;
		miny = miny < 0 ? 0 : miny;
		maxy = maxy >= height ? height-1 : maxy;
		
		int numopen = 0;

		for(int x = minx; x <= maxx; x++) {
			for(int y = miny; y <= maxy; y++) {

				int place = 4 * ((y * height) + x);

				if(image[place+3] != 255) {
					openSpaces[numopen][0] = x;
					openSpaces[numopen][1] = y;
					numopen++;
					set = 1;
				}
			}
		}
		
		if(!set) {
			removeFromTree(closestNeighbour->location, closestNeighbour);			
		} else {
			//if there is only 1 space left the closest color will no longer be a valid neighbour after this color is placed
			if(numopen == 1) {
				removeFromTree(closestNeighbour->location, closestNeighbour);
			}

			int placement = (*pseudoRandom) % numopen;
			
			(*pseudoRandom)++;

			int place = 4 * ((openSpaces[placement][1] * height) + openSpaces[placement][0]);

			image[place] = color->r;
			image[place+1] = color->g;
			image[place+2] = color->b;
			image[place+3] = 255;

			color->x = openSpaces[placement][0];
			color->y = openSpaces[placement][1];

			putColorInTree(tree, color);
		}
	}

}

void placeFirstPixel(unsigned char* image, int width, int height, octtree* tree, supercolor* color) {
	int place = 4 * ((height * height / 2) + width / 2);

	image[place] = color->r;
	image[place+1] = color->g;
	image[place+2] = color->b;
	image[place+3] = 255;

	color->x = width / 2;
	color->y = height / 2;

	putColorInTree(tree, color);

	printf("placed first pixel\n");
}

int main() {

	setbuf(stdout, NULL);

	octtree* root = createOctTree(0, 0, 0, 256, 256, 256, NULL);

	supercolor* colors = (supercolor *) malloc(16777216 * sizeof(supercolor));

	int j = 0;
	for(int i = 0; i < 16777216; i++) {
		int r = (i & 0x00FF0000) >> 16;
		int g = (i & 0x0000FF00) >> 8;
		int b = i & 0x000000FF;
		colors[j++] = createSuperColor(r, g, b);
	}

	time_t start = time(0);

	srand(start);

	int width = 4096;
	int height = 4096;
	unsigned char* image = (unsigned char*) calloc(width * height, 4);

	int pseudoRandom = 0;

	printf("created colors\n");

	placeFirstPixel(image, width, height, root, &colors[0]);

	int todo = width * height;

	for(int i = 1; i < todo; i++) {

		int r = ((rand() & 0xFF) << 16) | (rand() & 0x0000FFFF);

		int random = i + (r % (16777216 - i));

		supercolor temp = colors[i];
		colors[i] = colors[random];
		colors[random] = temp;

		setPixel(image, width, height, &colors[i], root, &pseudoRandom);

		if (i % 100000 == 0) {
			time_t diff = time(0) - start;

			int s = diff;

			printf("\r%d%% complete, time taken %ds", (int)((i * 100) / (float)todo), s);
		}
	}

	time_t diff = time(0) - start;

	time_t now = time(0);
	char filename[50];

	sprintf(filename, "picture %lli - %llis.png", now, diff);

	outImage(filename, image, width, height);
}
