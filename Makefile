CC = gcc
CFLAGS = -pedantic -Werror -Og -g
LIBS = -lm
SRC = main.c
BIN = cache

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

.PHONY: clean

clean:
	rm -f $(BIN)

