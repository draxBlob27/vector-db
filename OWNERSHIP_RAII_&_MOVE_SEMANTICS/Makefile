f ?= main
STD = -std=c++23
W_FLAGS = -Wall -Werror -Wpedantic
D_FLAGS = -g -O0
SAN_FLAGS = -O1 -g -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer

all: run clean

hello:
	@echo "Hello Sanil Parmar"

easy: 
	clang++ ${STD} -o ${f} ${f}.cpp
	./${f}
	rm -rf ${f}

debug: compile_dbg
	lldb ./${f}
	rm -r ${f}.dSYM

compile_dbg:
	clang++ -std=gnu++14 -g ${f}.cpp -o ${f}

run: compile
	./${f}

compile:
	clang++ ${STD} ${W_FLAGS} -o ${f} ${f}.cpp

san: 
	clang++ ${STD} ${SAN_FLAGS} -o ${f} ${f}.cpp
	./${f}
	rm -rf ${f}.dSYM

clean:
	rm -f ${f}
	rm -r ${f}.dSYM

.PHONY:
	all hello run compile debug san easy clean