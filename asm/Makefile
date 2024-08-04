CC = g++
FLAGS = -g -Wall -Wextra -MMD -MP -O3
DIRS = includes
SRC_DIR = src/
INC_DIRS = ${addprefix -I, ${DIRS}}

OUTPUT_DIR = ${dirs}
OUTPUT_DEPS= ${dirs}

FILES_TO_COMPILE = ${foreach _D, ${SRC_DIR},${wildcard ${_D}*.cpp}}
OUTPUT_FILES_NAME = ${patsubst %.cpp, ${OUTPUT_DIR}%.o, ${FILES_TO_COMPILE}}
DEPS=${patsubst %.cpp, ${OUTPUT_DEPS}%.d, ${FILES_TO_COMPILE}}

-include ${DEPS}

usage:
	@echo "make all dirs=<Output_directory>"
	@echo "make clean dirs=<Output_directory>"
	
all: dir ${OUTPUT_FILES_NAME}
	${CC} ${FLAGS} ${OUTPUT_FILES_NAME} main.cpp ${INC_DIRS} -o ${OUTPUT_DIR}masm 

${OUTPUT_DIR}${SRC_DIR}%.o: ${SRC_DIR}%.cpp
	${CC} ${FLAGS} ${INC_DIRS} -c $< -o $@

${OUTPUT_DIR}%.o: %.cpp
	${CC} ${FLAGS} ${INC_DIRS} -c $< -o $@

dir:
	mkdir -p ${OUTPUT_DIR}${SRC_DIR}

.PHONY: dir  all usage clean directories

