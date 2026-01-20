f ?= main
all: clean

hello:
	@echo "Hello Sanil Parmar"

easy: run_easy
	rm -f ${f}

run_easy: compile_alter
	./${f}

compile_alter: 
	clang++ -std=c++23 -o ${f} ${f}.cpp

debug: clean_debug
	rm -f ${f}
	rm -r ${f}.dSYM

clean_debug: compile_dbg
	lldb ./${f}

compile_dbg:
	clang++ -std=gnu++14 -g ${f}.cpp -o ${f}

clean: run
	rm -f ${f}

run: compile
	./${f}

compile:
	clang++ -std=c++23 -Wall -Werror -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -o ${f} ${f}.cpp

san: clean_san

clean_san: run_san
	rm -f ${f}
	rm -r ${f}.dSYM

run_san: compile_san
	./${f}

compile_san:
	clang++ -O1 -g -std=c++23 -fsanitize=undefined,address -fno-omit-frame-pointer -o ${f} ${f}.cpp

.PHONY:
	all hello run compile