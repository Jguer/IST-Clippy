.PHONY: default all clean test
CC = gcc
# CFLAGS = -std=gnu11 -g -Wall -Wextra -Wundef \
#					 -Wshadow -Wfloat-equal -Wpointer-arith \
#					 -Wcast-align -Wstrict-prototypes -Wstrict-overflow=5 \
#					 -Wwrite-strings -Waggregate-return -Wcast-qual \
#					 -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code

LFLAGS = -lm
CFLAGS = -std=gnu11

default: xhelloworld xtestwait xfuzzer clippy
all: default

# Apps
xhelloworld: apps/helloworld/main.c clipboard.o
	$(CC) $(CFLAGS) $(LFLAGS) -o xhelloworld apps/helloworld/main.c clipboard.o
xtestwait: apps/testwait/main.c clipboard.o
	$(CC) $(CFLAGS) $(LFLAGS) -o xtestwait apps/testwait/main.c clipboard.o
xfuzzer: apps/fuzzer.c clipboard.o
	$(CC) $(CFLAGS) $(LFLAGS) -o xfuzzer apps/fuzzer.c clipboard.o

# Server
clippy:clippy/main.c clippy/comms.c clippy/comms.h \
	clippy/storage.c clippy/storage.h \
	log.o list.o clipboard.o
	$(CC) $(CFLAGS) $(LFLAGS) -lpthread -o xclippy clippy/main.c clippy/comms.c clippy/comms.h \
	clippy/storage.c clippy/storage.h \
	log.o list.o clipboard.o


# Dependencies
clipboard.o : library/clipboard.c library/clipboard.h
	$(CC) $(CFLAGS) $(LFLAGS) -c library/clipboard.c library/clipboard.h
log.o : utils/log.c utils/log.h
	$(CC) $(CFLAGS) $(LFLAGS) -c utils/log.c utils/log.h -DLOG_USE_COLOR
list.o : utils/list.c utils/list.h
	$(CC) $(CFLAGS) $(LFLAGS) -c utils/list.c utils/list.h

clean :
	-rm -f *.o
	-rm -f x*
