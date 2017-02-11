CC=gcc

MINFO_LIBS=pangocairo x11 xinerama
MINFO_CFLAGS=-Wall $(shell pkg-config --cflags $(MINFO_LIBS))
MINFO_LDFLAGS=$(shell pkg-config --libs $(MINFO_LIBS))
MINFO_SOURCES=src/minfo.c src/audio-module.c src/date-module.c src/frame-limiter.c src/graphics.c src/hub.c src/module.c src/module-type.c src/radio.c src/surface.c src/time-module.c src/time-utils.c

MINFO_MSG_CFLAGS=-Wall
MINFO_MSG_LDFLAGS=
MINFO_MSG_SOURCES=src/minfo-msg.c src/module-type.c src/radio.c

all: minfo minfo_msg

minfo: $(MINFO_SOURCES)
	@$(CC) $(MINFO_CFLAGS) $(MINFO_SOURCES) $(MINFO_LDFLAGS) -o minfo

minfo_msg: $(MINFO_MSG_SOURCES)
	@$(CC) $(MINFO_MSG_CFLAGS) $(MINFO_MSG_SOURCES) $(MINFO_MSG_LDFLAGS) -o minfo-msg
