CC = gcc
CFLAGS = -lGLEW -lglfw -lGL -lm

BUILD_DIR = bin

all:

%: %.c
	$(CC) $< -o $(BUILD_DIR)/$@ $(CFLAGS)

run_%: $(BUILD_DIR)/%
	./$<

clean:
	rm -rf bin/*

PHONY: all run clean
