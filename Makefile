all:
	gcc -o t ttf.c -I /usr/local/include/freetype2/ -lfreetype -lm
