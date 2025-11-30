.PHONY: run build test clean

run:
	./a.out
build:
	gcc -std=c99 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined src/allocator.c -o bin/allocator

test:
	gcc -std=c99  src/allocator.c test/test.c -o bin/test

clean:
	rm -r bin/*

.DEFAULT_GOAL := build 