.PHONY: run build test

run:
	./a.out
build:
	gcc -std=c99 src/main.c src/allocator.c

test:
	gcc -std=c99  src/allocator.c test/test.c -o bin/test

.DEFAULT_GOAL := build 