all: 
	gcc -std=c99 -framework Glut -framework OpenGL lodepng.c allcolors.c -o allcolors