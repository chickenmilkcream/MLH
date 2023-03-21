CC := g++
CFLAGS := -Wall -g
OBJFILES := avl_node.o avl_tree.o memtable.o kv_store.o xxhash.o ClockBitmap.o LRUCache.o bp_pageframe.o bp_linkedlist.o bp_directory.o

all: clean phase2_clock

oof: $(OBJFILES) phase1_experiments.o
	$(CC) $(CFLAGS) -o phase1_experiments $(OBJFILES) phase1_experiments.o
	./phase1_experiments

phase1_tests: $(OBJFILES) phase1_tests.o
	$(CC) $(CFLAGS) -o phase1_tests $(OBJFILES) phase1_tests.o
	./phase1_tests

phase2_tests: $(OBJFILES) phase2_tests.o
	$(CC) $(CFLAGS) -o phase2_tests $(OBJFILES) phase2_tests.o
	./phase2_tests

phase2_clock: clean $(OBJFILES) phase2_test_clock.o
	$(CC) $(CFLAGS) -o phase2_test_clock $(OBJFILES) phase2_test_clock.o
	./phase2_test_clock

phase2_lru: clean $(OBJFILES) phase2_test_lru.o
	$(CC) $(CFLAGS) -o phase2_test_lru $(OBJFILES) phase2_test_lru.o
	./phase2_test_lru



%.o: %.cpp %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o main *.bin

#test: clean
#	$(CC) $(CFLAGS) -o $(OBJFILES)
#	./tests