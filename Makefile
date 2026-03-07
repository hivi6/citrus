BIN_NAME  := citrus
BUILD_DIR := build
BIN_PATH  := $(BUILD_DIR)/$(BIN_NAME)

C_FILES := $(wildcard src/*.c)
H_FILES := $(wildcard include/*.c)

C_FLAGS ?=

$(BIN_PATH): $(BUILD_DIR) $(C_FILES) $(H_FILES)
	$(CC) -Iinclude $(C_FLAGS) -o $(BIN_PATH) $(C_FILES)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

