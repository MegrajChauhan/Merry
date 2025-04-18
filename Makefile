# Variable definitions
CC = gcc
FLAGS = -Wall -Wextra -MMD -MP -O3
DIRS = utils
SRC_DIR = merry/
INC_DIRS = ${addprefix -I, ${DIRS}}
FLAGS += ${flags}

OUTPUT_DIR = ${dirs}
OUTPUT_DEPS= ${dirs}

FILES_TO_COMPILE = ${foreach _D, ${SRC_DIR},${wildcard ${_D}*.c}}
OUTPUT_FILES_NAME = ${patsubst %.c, ${OUTPUT_DIR}%.o, ${FILES_TO_COMPILE}}
DEPS=${patsubst %.c, ${OUTPUT_DEPS}%.d, ${FILES_TO_COMPILE}}

usage:
	@echo "make all dirs=<Output_directory>"
	@echo "make clean dirs=<Output_directory>"
	
all: directories ${OUTPUT_FILES_NAME}
	${CC} ${FLAGS} ${OUTPUT_FILES_NAME} main.c ${INC_DIRS} -o ${OUTPUT_DIR}mvm
# ${CC} ${FLAGS} ${OUTPUT_FILES_NAME} childmain.c merry/arithmetic.S ${INC_DIRS} -o ${OUTPUT_DIR}cmain
# ${CC} -ffunction-sections -fdata-sections -Wl,--gc-sections ${FLAGS} ${OUTPUT_FILES_NAME} merry_submain.c ${INC_DIRS} -o ${OUTPUT_DIR}subsysmain
# make -C asm all dirs=../build/ flags=${flags}

# The command for building the assembler is the simplest for now but hey! it works!

${OUTPUT_DIR}${SRC_DIR}%.o: ${SRC_DIR}%.c 
	${CC} ${FLAGS} ${INC_DIRS} -c $< -o $@

${OUTPUT_DIR}%.o: %.c
	${CC} ${FLAGS} ${INC_DIRS} -c $< -o $@

# Create necessary directories
directories:
	mkdir -p ${OUTPUT_DIR}
	${foreach f, ${SRC_DIR}, ${shell mkdir -p ${OUTPUT_DIR}${f}}}

clean:
	rm -rf ${OUTPUT_DIR}

# __pretest:
# ifeq ($(OS),Windows_NT)
#     DIRS += merry/abs/win
# 	@echo Windows is not currently fully supported :(
# 	@exit
# 	INC_DIRS += ${addprefix -I, merry/abs/win}
# else
#     UNAME_S := $(shell uname -s)
#     ifeq ($(UNAME_S),Linux)
# 		DIRS += merry/abs/linux
# 		INC_DIRS += ${addprefix -I, merry/abs/linux}
#     endif
# endif

.PHONY: all usage clean directories

-include $(DEPS)
