OBJS = $(wildcard *.cpp)
CC = g++
INCLUDE_PATHS = -I/usr/local/include -I/opt/homebrew/include/SDL2
LIBRARY_PATHS = -L/usr/local/lib -L/opt/homebrew/lib
COMPILER_FLAGS = -std=c++11 -Wall -O2 -D_THREAD_SAFE
LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
OBJ_NAME = exe_name

all:
	$(CC) -o $(OBJ_NAME) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) $(OBJS)