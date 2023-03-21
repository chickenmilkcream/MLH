CC := g++
CFLAGS := -Wall -g
OBJFILES := avl_node.o avl_tree.o memtable.o kv_store.o

all: clean phase1_tests

phase1_experiments: $(OBJFILES) phase1_experiments.o
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
	rm -f *.o main *_experiments *_tests *.bin

#test: clean
#	$(CC) $(CFLAGS) -o $(OBJFILES)
#	./tests