CC=gcc
CFLAGS=-Wall -Wextra -Werror -pthread

ifdef DEBUG
CFLAGS += -DDEBUG
endif

all: wordcount

wordcount: main.o scanner.o hashmap.o
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c wordcount.h
	$(CC) $(CFLAGS) -c $<

scanner.o: scanner.c wordcount.h
	$(CC) $(CFLAGS) -c $<

hashmap.o: hashmap.c wordcount.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o wordcount

.PHONY: all clean
