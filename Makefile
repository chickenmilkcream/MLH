CC := g++
CFLAGS := -Wall -g
OBJFILES := avl_node.o avl_tree.o memtable.o kv_store.o bp_pageframe.o bp_linkedlist.o bp_directory.o

all: clean phase2_tests

oof: $(OBJFILES) phase1_experiments.o
	$(CC) $(CFLAGS) -o phase1_experiments $(OBJFILES) phase1_experiments.o
	./phase1_experiments

phase1_tests: $(OBJFILES) phase1_tests.o
	$(CC) $(CFLAGS) -o phase1_tests $(OBJFILES) phase1_tests.o
	./phase1_tests

phase2_tests: $(OBJFILES) phase2_tests.o
	$(CC) $(CFLAGS) -o phase2_tests $(OBJFILES) phase2_tests.o
	./phase2_tests

%.o: %.cpp %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o main *.bin

#test: clean
#	$(CC) $(CFLAGS) -o $(OBJFILES)
#	./tests