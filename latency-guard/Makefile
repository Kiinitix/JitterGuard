CC=gcc
CFLAGS=-O2 -Wall -Wextra -std=gnu11

BIN_DIR=.
BENCH_DIR=bench

all: fg_probe analyze

fg_probe:
	$(CC) $(CFLAGS) $(BENCH_DIR)/fg_probe.c -o $(BIN_DIR)/fg_probe

analyze:
	$(CC) $(CFLAGS) $(BENCH_DIR)/analyze.c -o $(BIN_DIR)/analyze

clean:
	rm -f fg_probe analyze
