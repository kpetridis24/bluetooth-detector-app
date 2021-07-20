CC=gcc
CFLAGS=-O3 -lpthread
BUILD_DIR=build
SRC_DIR=src
INCLUDE_DIR=./include
SOURCES := $(shell find $(SRC_DIR) -name '*.c')

$(info $(shell mkdir -p $(BUILD_DIR)))

default:
	$(CC) -o $(BUILD_DIR)/main -I$(INCLUDE_DIR) $(SOURCES) $(CFLAGS)
	./build/main

clean:
	rm test