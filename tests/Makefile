CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -g
LDFLAGS = -lglfw -lGLESv2 -lGLEW -lm

BUILD_DIR = build
INC_DIR = include

TEST_SRCS = $(wildcard *.c)

TEST_OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(TEST_SRCS))

TARGET = $(BUILD_DIR)/robustness_suite

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(TEST_OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)/*
