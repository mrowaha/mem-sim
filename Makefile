CC=gcc
CFLAGS=-Wall -g
SFLAGS=-I./src/include
.DEFAULT_GOAL=all

SIMULATOR=memsim
LEVEL=2
ADDRFILE=./in/addrfile
SWAPFILE=swapfile.bin
FCOUNT=47
ALGO=FIFO
TICK=2
OUTFILE=./out/outfile

all: sim-build

sim-build: ./src/*.c
	$(CC) ./src/*.c -o ./bin/memsim $(CFLAGS) $(SFLAGS)

sim-run:
	./bin/memsim -p $(LEVEL) -r $(ADDRFILE) -s $(SWAPFILE) -f $(FCOUNT) -a $(ALGO) -t $(TICK) -o $(OUTFILE)

sim-leak:
	valgrind --leak-check=yes ./bin/memsim -p $(LEVEL) -r $(ADDRFILE) -s $(SWAPFILE) -f $(FCOUNT) -a $(ALGO) -t $(TICK) -o $(OUTFILE)

clean:
	rm -f ./bin/* $(SWAPFILE)