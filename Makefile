PRGNAME     = DiviDead.elf
CC          = gcc
CXX 		= g++

VIDEO_FORMAT = ROQ
PORT 		 = RS90
GIT_VERSION := " $(shell git rev-parse --short HEAD || echo unknown)"

INCLUDES	= -I./ -Isrc -Isrc/smpeg -IRES -Isrc/roq

SRCDIR 		=  ./src ./src/roq ./RES
ifeq ($(PORT), RS97)
DEFINES		= -DRS97
else ifeq ($(PORT), RS90)
DEFINES		= -DRS90
else ifeq ($(PORT), GCW0)
DEFINES		= -DGCW0
else ifeq ($(PORT), BITTBOY)
DEFINES		= -DBITTBOY
endif

CFLAGS		= -O0 -g3 -march=native -fno-common -Wall $(INCLUDES) $(DEFINES)
CXXFLAGS	= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++98
LDFLAGS     = -lc -lgcc -lm -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf -lm -lstdc++ -lz -pthread -lportaudio

# Files to be compiled
SRCDIR 		=  ./src ./RES
ifeq ($(VIDEO_FORMAT), ROQ)
DEFINES		+= -DENABLE_VIDEO_ROQ
SRCDIR 		+= ./src/roq
else
DEFINES		+= -DENABLE_VIDEO_SMPEG
SRCDIR 		+= ./src/smpeg ./src/smpeg/video ./src/smpeg/audio
endif
VPATH		= $(SRCDIR)
SRC_C		= $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.c))
SRC_CPP		= $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.cpp))
OBJ_C		= $(notdir $(patsubst %.c, %.o, $(SRC_C)))
OBJ_CPP		= $(notdir $(patsubst %.cpp, %.o, $(SRC_CPP)))
OBJS		= $(OBJ_C) $(OBJ_CPP)

# Rules to make executable
$(PRGNAME): $(OBJS)  
	$(CC) $(CFLAGS) -std=gnu99 -o $(PRGNAME) $^ $(LDFLAGS)
	
$(OBJ_CPP) : %.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(PRGNAME) *.o

clean_all:
	rm -f $(PRGNAME) *.o RES/*.c RES/*.CHARS RES/SPLASH.JPG RES/END/*.lz
