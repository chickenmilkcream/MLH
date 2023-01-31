CC := g++
CFLAGS := -Wall -g
binaries=memtable

all: $(TARGET)

%: %.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o $(binaries)
