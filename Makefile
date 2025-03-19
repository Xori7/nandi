ifneq ($(OS),Windows_NT)
	UNAME_S = $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		OS = Linux
	endif
endif

#Config
GAME_NAME = nandi
SRC = src
BUILD = build
OBJ = obj
TEST = test
#_Config

CC = gcc
C_FLAGS = -g -Wall -Wconversion -Wpedantic -Werror -std=c11

ifeq ($(OS),Windows_NT)
	LIBS = -L$(VULKAN_SDK)/Lib -lvulkan-1
else ifeq ($(OS),Linux)
	LIBS = -L$(VULKAN_SDK)/Lib -lvulkan
endif

TEST_LIBS = -L$(BUILD)/$(OS) -lnandi

INCLUDES = $(subst \,/,-I$(VULKAN_SDK)/Include)
INCLUDES += -Iinclude

SRC_FILES = $(wildcard $(SRC)/*.c) $(wildcard $(SRC)/**/*.c) $(wildcard $(SRC)/**/**/*.c) $(wildcard $(SRC)/**/**/**/*.c) $(wildcard $(SRC)/**/**/**/**/*.c)
TEST_SRC_FILES = $(wildcard $(TEST)/*.c) $(wildcard $(TEST)/**/*.c) $(wildcard $(TEST)/**/**/*.c) $(wildcard $(TEST)/**/**/**/*.c) $(wildcard $(TEST)/**/**/**/**/*.c)
 
OBJ_FILES = $(patsubst $(SRC)/%.c, $(BUILD)/$(OS)/$(OBJ)/%.o, $(SRC_FILES))
TEST_OBJ_FILES = $(filter-out $(BUILD)/$(OS)/$(OBJ)/main.o, $(OBJ_FILES)) $(patsubst $(TEST)/%.c, $(BUILD)/$(OS)/$(TEST)/%.o, $(TEST_SRC_FILES))

ifeq ($(OS),Windows_NT)
	TARGET = $(BUILD)/$(OS)/$(GAME_NAME).dll
else ifeq ($(OS),Linux)
	TARGET = $(BUILD)/$(OS)/$(GAME_NAME).so
endif

TEST_TARGET = $(BUILD)/$(OS)/$(TEST)/$(GAME_NAME)_test

build: $(TARGET)

test: build $(TARGET) build $(TEST_TARGET)
	./$(TEST_TARGET)

tidy: 
	clang-tidy $(SRC_FILES) -- $(INCLUDES) -checks=-*

init: 
	rm -f ./compile_flags.txt
	printf "$(subst -,\n-,$(INCLUDES))" > ./compile_flags.txt
	mkdir debug/

$(TARGET): $(OBJ_FILES) | $(BUILD)/$(OS)
	$(CC) $(C_FLAGS) --shared -o $@ $^ $(LIBS)

$(TEST_TARGET): $(TEST_OBJ_FILES) | $(BUILD)/$(OS)/$(TEST)
	$(CC) $(C_FLAGS) -o $@ $^ $(TEST_LIBS)

$(BUILD)/$(OS)/$(OBJ)/%.o: $(SRC)/%.c | $(BUILD)/$(OS)/$(OBJ)
	mkdir -p $(dir $@)
	$(CC) -c $< $(C_FLAGS) $(INCLUDES) -o $@

$(BUILD)/$(OS)/$(TEST)/%.o: $(TEST)/%.c | $(BUILD)/$(OS)/$(TEST)
	mkdir -p $(dir $@)
	$(CC) -c $< $(C_FLAGS) $(INCLUDES) -o $@

$(BUILD)/$(OS): 
	mkdir -p $(BUILD)/$(OS)

$(BUILD)/$(OS)/$(OBJ): 
	mkdir -p $(BUILD)/$(OS)/$(OBJ)

$(BUILD)/$(OS)/$(TEST):
	mkdir -p $(BUILD)/$(OS)/$(TEST)

clean: 
	rm -r ./debug/
	rm -r $(BUILD)/*

.PHONY : all clean

