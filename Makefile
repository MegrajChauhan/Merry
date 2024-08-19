# Variable definitions
CC = gcc
FLAGS = -Wall -Wextra -MMD -MP -O3
DIRS = merry/core merry/defs merry/dev merry/os merry/tools utils
SRC_DIR = merry/abs/ merry/
INC_DIRS = ${addprefix -I, ${DIRS}}

OUTPUT_DIR = ${dirs}
OUTPUT_DEPS= ${dirs}

FILES_TO_COMPILE = ${foreach _D, ${SRC_DIR},${wildcard ${_D}*.c}}
OUTPUT_FILES_NAME = ${patsubst %.c, ${OUTPUT_DIR}%.o, ${FILES_TO_COMPILE}}
DEPS=${patsubst %.c, ${OUTPUT_DEPS}%.d, ${FILES_TO_COMPILE}}

usage:
	@echo "make all dirs=<Output_directory>"
	@echo "make clean dirs=<Output_directory>"
	
all: __pretest directories ${OUTPUT_FILES_NAME}
	${CC} ${FLAGS} ${OUTPUT_FILES_NAME} main.c merry/arithmetic.S ${INC_DIRS} -o ${OUTPUT_DIR}mvm
	g++ -Iasm/includes asm/src/* asm/main.cpp -o ${OUTPUT_DIR}masm

# The command for building the assembler is the simplest for now but hey! it works!

${OUTPUT_DIR}${SRC_DIR}%.o: ${SRC_DIR}%.c 
	${CC} ${FLAGS} ${INC_DIRS} -c $< -o $@

${OUTPUT_DIR}%.o: %.c
	${CC} ${FLAGS} ${INC_DIRS} -c $< -o $@

# Create necessary directories
directories:
	mkdir -p ${OUTPUT_DIR}
	${foreach f, ${SRC_DIR}, ${shell "mkdir -p ${OUTPUT_DIR}${f}"}}

clean:
	rm -rf ${OUTPUT_DIR}

__pretest:
ifeq ($(OS),Windows_NT)
    DIRS += merry/abs/win
	INC_DIRS += ${addprefix -I, merry/abs/win}
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
		DIRS += merry/abs/linux
		INC_DIRS += ${addprefix -I, merry/abs/linux}
    endif
endif

.PHONY: __pretest all usage clean directories

-include $(DEPS)
