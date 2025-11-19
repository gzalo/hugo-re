# Makefile for Hugo Lite

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 -Wno-unused-parameter
LIBS = -lSDL2 -lSDL2_image -lSDL2_mixer -lm

# Detect SDL2 using pkg-config
SDL2_CFLAGS = $(shell pkg-config --cflags sdl2 SDL2_image SDL2_mixer)
SDL2_LIBS = $(shell pkg-config --libs sdl2 SDL2_image SDL2_mixer)

TARGET = hugo_lite
SRC = hugo_lite.c cave.c forest.c state.c cJSON.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SDL2_CFLAGS) -o $(TARGET) $(SRC) $(SDL2_LIBS) -lm

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
