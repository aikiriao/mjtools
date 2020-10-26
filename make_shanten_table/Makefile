CC=gcc
CFLAGS=-Wall -Wextra
TARGET=make_shanten_table

all: $(TARGET)

clean:
	rm $(TARGET)

make_shanten_table: make_shanten_table.c
	$(CC) $(CFLAGS) $< -o $@
