CC := g++
CFLAGS := -Wall -g
OBJFILES := avl_node.o avl_tree.o memtable.o kv_store.o xxhash.o ClockBitmap.o LRUCache.o bp_pageframe.o bp_linkedlist.o bp_directory.o

all: clean phase2_clock

phase1_experiments: $(OBJFILES) experiments/phase1_experiments.o
	$(CC) $(CFLAGS) -o experiments/phase1_experiments $(OBJFILES) experiments/phase1_experiments.o
	./experiments/phase1_experiments

phase1_tests: $(OBJFILES) phase1_tests.o
	$(CC) $(CFLAGS) -o phase1_tests $(OBJFILES) phase1_tests.o
	./phase1_tests

phase2_tests: $(OBJFILES) phase2_tests.o
	$(CC) $(CFLAGS) -o phase2_tests $(OBJFILES) phase2_tests.o
	./phase2_tests

phase2_clock: clean $(OBJFILES) phase2_clock_tests.o clean
	$(CC) $(CFLAGS) -o phase2_clock_tests $(OBJFILES) phase2_clock_tests.o
	./phase2_clock_tests

phase2_lru: clean $(OBJFILES) phase2_lru_tests.o clean
	$(CC) $(CFLAGS) -o phase2_lru_tests $(OBJFILES) phase2_lru_tests.o
	./phase2_lru_tests

all_tests: clean phase1_tests clean phase2_tests clean phase2_clock clean phase2_lru clean

%.o: %.cpp %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o experiments/*.o main experiments/*_experiments *_tests *.bin

