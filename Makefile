CC       := gcc
CFLAGS   := -Wall -Wextra -std=c11 -O2 -Iinclude
SRC      := src/main.c src/page_table.c src/tlb.c src/backing_store.c
OBJ      := $(SRC:.c=.o)
TARGET   := bin/vmsim

.PHONY: all clean run addresses

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $(OBJ)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

addresses:
	python3 scripts/generate_addresses.py > data/addresses.txt

run: all
	./$(TARGET) data/addresses.txt lru -v

clean:
	rm -f src/*.o $(TARGET) data/BACKING_STORE.bin
