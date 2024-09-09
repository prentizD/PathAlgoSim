CC = gcc
CFLAGS = -O1 -Wall -std=c99 -Wno-missing-braces
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
TARGET = simulation
BUILD_DIR = build

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): main.c
	mkdir -p ${BUILD_DIR}
	$(CC) $(CFLAGS) main.c -o $@ $(LIBS)

clean:
	rm -rf $(BUILD_DIR)