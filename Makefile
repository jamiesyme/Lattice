CC=gcc

LATTICE_LIBS=pangocairo x11 xinerama
LATTICE_CFLAGS=-Wall -lm $(shell pkg-config --cflags $(LATTICE_LIBS))
LATTICE_LDFLAGS=$(shell pkg-config --libs $(LATTICE_LIBS))
LATTICE_SOURCES=src/lattice.c src/audio-module.c src/command-utils.c src/date-module.c src/draw-utils.c src/frame-limiter.c src/hub.c src/interpolation-utils.c src/json.c src/module.c src/module-director.c src/module-renderer.c src/module-type.c src/radio.c src/surface.c src/time-module.c src/time-utils.c src/workspace-module.c

LATTICE_MSG_CFLAGS=-Wall
LATTICE_MSG_LDFLAGS=
LATTICE_MSG_SOURCES=src/lattice-msg.c src/module-type.c src/radio.c

all: lattice lattice_msg

lattice: $(LATTICE_SOURCES)
	@$(CC) $(LATTICE_CFLAGS) $(LATTICE_SOURCES) $(LATTICE_LDFLAGS) -o lattice

lattice_msg: $(LATTICE_MSG_SOURCES)
	@$(CC) $(LATTICE_MSG_CFLAGS) $(LATTICE_MSG_SOURCES) $(LATTICE_MSG_LDFLAGS) -o lattice-msg

clean:
	@rm -f lattice lattice-msg
