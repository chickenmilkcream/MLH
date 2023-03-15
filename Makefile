CC := g++
CFLAGS := -Wall -g
OBJFILES := avl_node.o avl_tree.o memtable.o kv_store.o

all: clean phase1_tests

phase1_experiments: $(OBJFILES) experiments/phase1_experiments.o
	$(CC) $(CFLAGS) -o experiments/phase1_experiments $(OBJFILES) experiments/phase1_experiments.o
	./experiments/phase1_experiments

phase1_tests: $(OBJFILES) phase1_tests.o
	$(CC) $(CFLAGS) -o phase1_tests $(OBJFILES) phase1_tests.o
	./phase1_tests

%.o: %.cpp %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o experiments/*.o main experiments/*_experiments *_tests *.bin
