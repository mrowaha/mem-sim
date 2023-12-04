CC=gcc
CFLAGS=-Wall -g
SFLAGS=-I./src/include
BUILD_DIR := ./bin
.DEFAULT_GOAL=all

SIMULATOR=memsim
LEVEL=2
ADDRFILE=testfile.txt
SWAPFILE=swapfile.bin
FCOUNT=4
ALGO=LRU
TICK=2
OUTFILE=outfile.txt

all: clean build run
 
build: ./src/*.c
	@$(CC) ./src/*.c -o ./bin/memsim $(CFLAGS) $(SFLAGS)

run:
	./bin/memsim -p $(LEVEL) -r $(ADDRFILE) -s $(SWAPFILE) -f $(FCOUNT) -a $(ALGO) -t $(TICK) -o $(OUTFILE)

leak:
	@valgrind --leak-check=yes ./bin/memsim -p $(LEVEL) -r $(ADDRFILE) -s $(SWAPFILE) -f $(FCOUNT) -a $(ALGO) -t $(TICK) -o $(OUTFILE)

clean:
	@rm -f ./bin/* $(SWAPFILE)

test-pagetable:
	@gcc ./src/pagetable.c ./tests/pagetabletest.c -o ./bin/pagetabletest -I./src/include -lcriterion

test-dblpagetable:
	@gcc ./src/pagetable.c ./tests/dblpagetabletest.c -o ./bin/dblpagetabletest -I./src/include -lcriterion
