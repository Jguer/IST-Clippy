.PHONY: default all clean test
CC = gcc
# CFLAGS = -std=gnu11 -g -Wall -Wextra -Wundef \
			        # -Wshadow -Wfloat-equal -Wpointer-arith \
					# -Wcast-align -Wstrict-prototypes -Wstrict-overflow=5 \
					# -Wwrite-strings -Waggregate-return -Wcast-qual \
					# -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code

CFLAGS = -std=gnu11 -g -Wall

LFLAGS = -lm
# CFLAGS = -std=gnu11 -O2 -Os

default: apps server
all: default

apps: xhelloworld xtestwait xfuzzer xminifuzzer xtypetoclippy
server: clippy

# Apps
xhelloworld: apps/helloworld/main.c clipboard.o
	$(CC) $(CFLAGS) -o xhelloworld apps/helloworld/main.c clipboard.o $(LFLAGS)
xtestwait: apps/testwait/main.c clipboard.o
	$(CC) $(CFLAGS) -o xtestwait apps/testwait/main.c clipboard.o $(LFLAGS)
xfuzzer: apps/fuzzer.c clipboard.o log.o
	$(CC) $(CFLAGS) -o xfuzzer apps/fuzzer.c clipboard.o log.o $(LFLAGS)
xminifuzzer: apps/minifuzzer.c clipboard.o log.o
	$(CC) $(CFLAGS) -o xminifuzzer apps/minifuzzer.c clipboard.o log.o $(LFLAGS)
xtypetoclippy: apps/typetoclippy.c clipboard.o log.o
	$(CC) $(CFLAGS) -o xtypetoclippy apps/typetoclippy.c clipboard.o log.o $(LFLAGS)

# Server
clippy:clippy/main.c clippy/comms.c clippy/comms.h \
	clippy/storage.c clippy/storage.h \
	log.o list.o clipboard.o
	$(CC) $(CFLAGS) -o xclippy clippy/main.c clippy/comms.c clippy/comms.h \
	clippy/storage.c clippy/storage.h \
	log.o list.o clipboard.o $(LFLAGS) -lpthread


# Dependencies
clipboard.o : library/clipboard.c library/clipboard.h
	$(CC) $(CFLAGS) -c library/clipboard.c library/clipboard.h $(LFLAGS)
log.o : utils/log.c utils/log.h
	$(CC) $(CFLAGS) -c utils/log.c utils/log.h -DLOG_USE_COLOR $(LFLAGS)
list.o : utils/list.c utils/list.h
	$(CC) $(CFLAGS) -c utils/list.c utils/list.h $(LFLAGS)

clean :
	-rm -f *.o
	-rm -f x*
