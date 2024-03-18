CC=gcc
CFLAGS = -Wall
level_1.1:level_1.c xstring.c xargparse.c
	gcc level_1.c xstring.c xargparse.c -o level_1.1 -L. -lvideodecoder -lavformat -lavcodec -lavutil -lswscale
