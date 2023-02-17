CC := g++
CFLAGS := -Wall -g

all: clean kv_store memtable

avlnode: avlnode.cpp
	$(CC) -o avlnode avlnode.cpp

memtable: memtable.cpp
	$(CC) -o memtable memtable.cpp

kv_store: kv_store.cpp
	$(CC) -o kv_store kv_store.cpp

main: main.cpp
	$(CC) -o main main.cpp

clean:
	rm -rf *.o memtable
