UNAME := $(shell uname)



INCDIR = ../../Engine ../../Common
LIBDIR =
CC = gcc
CXX = g++
LIBS = -lm -lstdc++

ifeq ($(UNAME), Darwin)

LIBS += -framework SDL2 -framework SDL2_mixer
SDL_LIB = -F/Library/Frameworks -F/Library/Frameworks/SDL2_mixer.framework/Versions/A/
SDL_INCLUDE = -F/Library/Frameworks -I/Library/Frameworks/SDL2.framework/Headers/ -I/Library/Frameworks/SDL2_mixer.framework/Versions/A/Headers/

CFLAGS = -fPIC -fvisibility=hidden -O2 -g -Wall $(SDL_INCLUDE)
LDFLAGS = $(SDL_LIB)
TARGET = libagswaves.dylib
CFLAGS += -DMAC_VERSION
else

SDL_LIB = `sdl2-config --libs` -lSDL2_mixer
SDL_INCLUDE = `sdl2-config --cflags`

CFLAGS = -fPIC -fvisibility=hidden -O2 -g -Wall $(SDL_INCLUDE)
LDFLAGS = $(SDL_LIB)
TARGET = libagswaves.so
CFLAGS += -DLINUX_VERSION
endif

CXXFLAGS = $(CFLAGS)

include Makefile-objs


CFLAGS   := $(addprefix -I,$(INCDIR)) $(CFLAGS)
CXXFLAGS := $(CFLAGS) $(CXXFLAGS)
ASFLAGS  := $(CFLAGS) $(ASFLAGS)


.PHONY: clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CC) -shared -dynamiclib -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIBS)

%.o: %.c
	@echo $@
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	@echo $@
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	@rm -f $(TARGET) $(OBJS)
