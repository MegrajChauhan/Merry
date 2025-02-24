# REUSE OF THE MAKEFILE FOR THE VM

# Variable definitions
CC = g++
FLAGS = -Wall -Wextra -MMD -MP -O3 -std=c++20
DIRS = includes
SRC_DIR = src/
INC_DIRS = ${addprefix -I, ${DIRS}}
INC_DIRS += -I../utils -I../merry/core
FLAGS += ${flags}

OUTPUT_DIR = ${dirs}
OUTPUT_DEPS= ${dirs}

FILES_TO_COMPILE = ${foreach _D, ${SRC_DIR},${wildcard ${_D}*.cpp}}
OUTPUT_FILES_NAME = ${patsubst %.cpp, ${OUTPUT_DIR}%.o, ${FILES_TO_COMPILE}}
DEPS=${patsubst %.cpp, ${OUTPUT_DEPS}%.d, ${FILES_TO_COMPILE}}

usage:
	@echo "make all dirs=<Output_directory>"
	@echo "make clean dirs=<Output_directory>"
	
all: directories ${OUTPUT_FILES_NAME}
	${CC} ${FLAGS} ${OUTPUT_FILES_NAME} main.cpp ${INC_DIRS} -o ${OUTPUT_DIR}masm

# The command for building the assembler is the simplest for now but hey! it works!

${OUTPUT_DIR}${SRC_DIR}%.o: ${SRC_DIR}%.cpp 
	${CC} ${FLAGS} ${INC_DIRS} -c $< -o $@

${OUTPUT_DIR}%.o: %.cpp
	${CC} ${FLAGS} ${INC_DIRS} -c $< -o $@

# Create necessary directories
directories:
	${foreach f, ${SRC_DIR}, ${shell mkdir -p ${OUTPUT_DIR}${f}}}

clean:
	rm -rf ${OUTPUT_DIR}

.PHONY: all usage clean directories

-include $(DEPS)
