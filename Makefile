CC := g++
CFLAGS := -Wall -g
OBJFILES := main.o kv_store.o memtable.o

all: main

main: $(OBJFILES)
	$(CC) $(CFLAGS) -o main $(OBJFILES)

%.o: %.cpp %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o main
