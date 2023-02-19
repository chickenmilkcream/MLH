CC := g++
CFLAGS := -Wall -g
OBJFILES := avl_node.o avl_tree.o memtable.o kv_store.o main.o

all: clean main

main: $(OBJFILES)
	$(CC) $(CFLAGS) -o main $(OBJFILES)
	./main

%.o: %.cpp %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o main *.bin
