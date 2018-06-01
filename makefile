.PHONY: default all clean test
CC = gcc
# CFLAGS = -std=gnu11 -g -Wall -Wextra -Wundef \
#								-Wshadow -Wfloat-equal -Wpointer-arith \
#						-Wcast-align -Wstrict-prototypes -Wstrict-overflow=5 \
#						-Wwrite-strings -Waggregate-return -Wcast-qual \
#						-Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code

# CFLAGS = -std=gnu11 -g -Wall

LFLAGS = -lm
CFLAGS = -std=gnu11 -O2 -Os

default: apps server
all: default

apps: xhelloworld xtestwait xfuzzer xminifuzzer xtypetoclippy
server: clippy

# Apps
xhelloworld: apps/helloworld.c clipboard.o
	$(CC) $(CFLAGS) -o xhelloworld apps/helloworld.c clipboard.o $(LFLAGS)
xtestwait: apps/testwait.c clipboard.o
	$(CC) $(CFLAGS) -o xtestwait apps/testwait.c clipboard.o $(LFLAGS)
xfuzzer: apps/fuzzer.c clipboard.o log.o
	$(CC) $(CFLAGS) -o xfuzzer apps/fuzzer.c clipboard.o log.o $(LFLAGS)
xminifuzzer: apps/minifuzzer.c clipboard.o log.o
	$(CC) $(CFLAGS) -o xminifuzzer apps/minifuzzer.c clipboard.o log.o $(LFLAGS)
xtypetoclippy: apps/typetoclippy.c clipboard.o log.o
	$(CC) $(CFLAGS) -o xtypetoclippy apps/typetoclippy.c clipboard.o log.o $(LFLAGS)

# Server
clippy: main.o storage.o comms.o log.o list.o clipboard.o
	$(CC) $(CFLAGS) -o xclippy main.o storage.o comms.o \
		log.o list.o clipboard.o $(LFLAGS) -lpthread


# Dependencies
main.o : clippy/main.c
	$(CC) $(CFLAGS) -c clippy/main.c clippy/comms.h
storage.o : clippy/storage.c clippy/storage.h
	$(CC) $(CFLAGS) -c clippy/storage.c clippy/storage.h
comms.o : clippy/comms.c clippy/comms.h
	$(CC) $(CFLAGS) -c clippy/comms.c clippy/comms.h
clipboard.o : library/clipboard.c library/clipboard.h
	$(CC) $(CFLAGS) -c library/clipboard.c library/clipboard.h
log.o : utils/log.c utils/log.h
	$(CC) $(CFLAGS) -c utils/log.c utils/log.h -DLOG_USE_COLOR
list.o : utils/list.c utils/list.h
	$(CC) $(CFLAGS) -c utils/list.c utils/list.h

clean :
	-rm -f *.o
	-rm -f x*
	-rm -f clippy/*.ghc
	-rm -f library/*.ghc
