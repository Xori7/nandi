LIB_NAME=nandi
CC=clang
BUILD_DIR=build
SRC_DIR=src
OUTPUT=$(BUILD_DIR)/$(LIB_NAME).lib
SRC_FILES=test.c cat.c
OBJ_FILES=$(patsubst %.c, $(BUILD_DIR)/%.o, $(SRC_FILES))

ifeq ($(OS),Windows_NT)
	C_COMP_FLAGS = -IC:/VulkanSDK/1.3.261.1/Include
	C_LINK_FLAGS = -LC:/VulkanSDK/1.3.261.1/Lib -lvulkan-1
endif


all : $(OUTPUT)

$(OUTPUT) : $(OBJ_FILES)
	ar rcs $(OUTPUT) $(OBJ_FILES)

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) -c $< $(C_COMP_FLAGS) -o $@

$(BUILD_DIR) :
	mkdir -p $(BUILD_DIR)

clean : 
	rm -r $(BUILD_DIR)/*

.PHONY : all clean
