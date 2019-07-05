PRGNAME     = DiviDead.elf
CC          = gcc
CXX 		= g++

GIT_VERSION := " $(shell git rev-parse --short HEAD || echo unknown)"

INCLUDES	= -I -Isrc -Isrc/smpeg -IRES

DEFINES		= -DENABLE_VIDEO_SMPEG -DBITTBOY

CFLAGS		= -O0 -g3 -march=native -fno-common -Wall $(INCLUDES) $(DEFINES)
CXXFLAGS	= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++98
LDFLAGS     = -lc -lgcc -lm -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf -lm -lstdc++ -lz -pthread -lportaudio

# Files to be compiled
SRCDIR 		=  ./src ./src/smpeg ./src/smpeg/video ./src/smpeg/audio ./RES
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
