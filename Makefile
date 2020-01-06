CC=gcc
CFLAGS=-I./src/ -I. -O1 -Wall -pedantic -std=c99
OBJ=build/system/event.o build/system/pools.o build/system/scheduler.o build/utils/circular-queue.o build/utils/closure.o build/utils/linked-list.o build/utils/object-pool.o
TEST_OBJ=build/test/utils/circular-queue.o build/test/utils/closure.o build/test/utils/linked-list.o
# TEST_OBJ=build/test/system/event.o build/test/system/pools.o build/test/system/scheduler.o build/test/utils/circular-queue.o build/test/utils/closure.o build/test/utils/linked-list.o build/test/utils/object-pool.o

dist/test: dist/libuevloop.so build/test.o $(TEST_OBJ)
	$(CC) -L$(shell pwd)/dist -o dist/test build/test.o $(TEST_OBJ) -luevloop $(CFLAGS)

build/test.o: test/test.c test/minunit.h
	$(CC) -c -o build/test.o test/test.c $(CFLAGS)

build/test/system/%.o: test/system/%.c test/system/%.h test/minunit.h
	mkdir -p build/test/system
	$(CC) -c -fpic -o $@ $< $(CFLAGS)

build/test/utils/%.o: test/utils/%.c test/utils/%.h test/minunit.h
	mkdir -p build/test/utils
	$(CC) -c -fpic -o $@ $< $(CFLAGS)

dist/libuevloop.so: $(OBJ)
	mkdir -p dist
	$(CC) -shared -fpic -o dist/libuevloop.so $(OBJ) $(CFLAGS)

build/system/%.o: src/system/%.c src/system/%.h
	mkdir -p build/system
	$(CC) -c -fpic -o $@ $< $(CFLAGS)

build/utils/%.o: src/utils/%.c src/utils/%.h
	mkdir -p build/utils
	$(CC) -c -fpic  -o $@ $< $(CFLAGS)

.PHONY: clean test

clean:
	rm -rf build dist

test:
	$(MAKE) && LD_LIBRARY_PATH=$(shell pwd)/dist:$(LD_LIBRARY_PATH) ./dist/test
