.PHONY: default all clean test
CC = gcc
default: xhelloworld xtestwait
all: default

xhelloworld: ./helloworld/main.c clipboard.o
	$(CC) -lm -o xhelloworld ./helloworld/main.c clipboard.o
xtestwait: ./testwait/main.c clipboard.o
	$(CC) -lm -o xtestwait ./testwait/main.c clipboard.o
clipboard.o : ../library/clipboard.c ../library/clipboard.h
	$(CC) -lm -c ../library/clipboard.c ../library/clipboard.h
clean :
	-rm -f *.o
	-rm -f x*
