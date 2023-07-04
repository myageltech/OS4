CC=g++
CFLAGS=-std=c++11

all: test_1 test_2 test_3

test_1: malloc_1.cpp test_1.cpp
	$(CC) $(CFLAGS) $^ -o $@

test_2: malloc_2.cpp test_2.cpp
	$(CC) $(CFLAGS) $^ -o $@

test_3: malloc_3.cpp
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f test_1 test_2 test_3
