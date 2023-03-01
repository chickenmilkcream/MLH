CC := g++
CFLAGS := -Wall -g
OBJFILES := avl_node.o avl_tree.o memtable.o kv_store.o

all: clean main

oof: $(OBJFILES) tests.o
	$(CC) $(CFLAGS) -o tests $(OBJFILES) tests.o
	./tests

main: $(OBJFILES) main.o
	$(CC) $(CFLAGS) -o main $(OBJFILES) main.o
	./main

%.o: %.cpp %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o main *.bin

#test: clean
#	$(CC) $(CFLAGS) -o $(OBJFILES)
#	./tests