# mem_alloc

A simple memory allocator in C using a linked list

## Architecture

### Basic concept
The memory is organized as a linked list of blocks:
```
[Header|Data][Header|Data][Header|Data]...
```

Each block contains:
- **size**: data size (without header)
- **is_free**: flag free/occupied
- **next**: pointer to the next block

### algorithms

1. **First Fit** - searching for the first suitable free block
2. **Splitting** - splitting a large block into two
3. **Coalescing** - merging adjacent free blocks

## Compilation and execution
make command are designed for unuix-like system
```bash
# Compilation
make
#or
gcc -std=c99 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined src/allocator.c -o bin/allocator

#Test
make test #compile test files
./bin/test

# Clean
make clean
```