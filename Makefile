####################################################################
##
##  
####################################################################

#############################################
# COMPILER FLAGS
################
SDL_FLAGS = `sdl-config --cflags --libs`
IPOD_FLAGS = -DIPOD -O2 -Wall -funroll-loops -finline-functions -elf2flt
HOTDOG_PATH ?= ../includes/
#############################################

ifdef IPOD
CROSS ?= arm-uclinux-elf
CC = $(CROSS)-gcc
LDFLAGS = keys.o $(HOTDOG_PATH)*.a -lm
FLAGS = $(IPOD_FLAGS)
else
CC = gcc
LD = ld
LDFLAGS = -lm
LDFLAGS += $(SDL_FLAGS)
FLAGS = -O2 -Wall
endif

ifdef SOUND
FLAGS += -DSOUND
endif
ifdef DEBUG
FLAGS += -DDEBUG
endif
ifdef DEEP_DEBUG
FLAGS += -DFUNCTION_DEBUG
endif
ifdef TEST
FLAGS += -DTEST
LDFLAGS += dct.o
endif


#########
# OPTIONS
#########
all:

mv_tools: mv_player mv_encoder mv_analyzer

mv_encoder: video.o audio.o
	$(CC) -o mv_encoder mv_encoder.c audio.o video.o $(FLAGS) $(LDFLAGS)

mv_player: video.o audio.o keys.o dct.o
	$(CC) -o mv_player mv_player.c audio.o video.o $(LDFLAGS) $(FLAGS)

mv_analyzer:
	$(CC) -o mv_analyzer mv_analyzer.c $(FLAGS) $(LDFLAGS)

audio.o: audio.c 
	$(CC) -o audio.o -c audio.c $(FLAGS)

keys.o: keyhandler.c 
	$(CC) -o keys.o -c keyhandler.c $(FLAGS)

video.o: video.c 
	$(CC) -o video.o -c video.c $(FLAGS)

dct.o: 
	$(CC) -c -o dct.o dct.S $(FLAGS)

clean:
	rm -f *.o
	rm -f *~
	rm -f *.gdb
	rm -f *.elf
	rm -f *.elf2flt
	rm -f ./test
	rm -f ./mv_analyzer
	rm -f ./mv_player
	rm -f ./mv_encoder
