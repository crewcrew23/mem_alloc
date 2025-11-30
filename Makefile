.PHONY: run build test

run:
	./a.out
build:
	gcc -std=c99 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined src/allocator.c

test:
	gcc -std=c99  src/allocator.c test/test.c -o bin/test

.DEFAULT_GOAL := build 