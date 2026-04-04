PROJECT = HexFlayer

CC = gcc
CFLAGS = -Wall -Wextra -std=c23

SRC_DIR = src
COMP_DIR = $(SRC_DIR)/components
BUILD_DIR = build

SRC = $(SRC_DIR)/main.c $(wildcard $(COMP_DIR)/*.c)

OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))
DEP = $(OBJ:.o=.d)

TARGET = $(BUILD_DIR)/$(PROJECT)

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

-include $(DEP)

.PHONY: all clean