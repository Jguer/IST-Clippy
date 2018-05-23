.PHONY: default all clean test
CC = gcc
default: xhelloworld xtestwait xfuzzer clippy
all: default

# Apps
xhelloworld: apps/helloworld/main.c clipboard.o
	$(CC) -lm -o xhelloworld apps/helloworld/main.c clipboard.o
xtestwait: apps/testwait/main.c clipboard.o
	$(CC) -lm -o xtestwait apps/testwait/main.c clipboard.o
xfuzzer: apps/fuzzer.c clipboard.o
	$(CC) -lm -o xfuzzer apps/fuzzer.c clipboard.o

# Server
clippy:clippy/main.c clippy/comms.c clippy/comms.h \
	clippy/storage.c clippy/storage.h \
	log.o list.o clipboard.o
	$(CC) -lm -lpthread -o xclippy clippy/main.c clippy/comms.c clippy/comms.h \
	clippy/storage.c clippy/storage.h \
	log.o list.o clipboard.o


# Dependencies
clipboard.o : library/clipboard.c library/clipboard.h
	$(CC) -c library/clipboard.c library/clipboard.h
log.o : utils/log.c utils/log.h
	$(CC) -c utils/log.c utils/log.h -DLOG_USE_COLOR
list.o : utils/list.c utils/list.h
	$(CC) -c utils/list.c utils/list.h

clean :
	-rm -f *.o
	-rm -f x*
