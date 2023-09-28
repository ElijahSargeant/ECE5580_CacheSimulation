CC = gcc
CFLAGS = -pedantic -Wall -Werror -Ofast -g
LIBS = -lm
SRC = Cache.Grp1.c
BIN = Cache.Grp1

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

.PHONY: clean

clean:
	rm -f $(BIN)

